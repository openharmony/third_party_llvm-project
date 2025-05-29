/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import java.time.LocalDateTime
import groovy.json.JsonOutput
import org.jenkinsci.plugins.workflow.steps.FlowInterruptedException
import net.sf.json.JSONObject


void withOhosDevice(Map params=[:], Closure body) {
    final int ERROR_FAIL_CONNECT_DEVICE = 111
    String rootDir = getSrcDir()
    if (!params.target_group) {
        error('[PND] Target_group should be passed!')
    }
    String targetGroup = params['target_group']
    String deviceSerial = params['DEVICE_SERIAL'] ?: env.LOCK_RESOURCE
    echo "Use Jenkins lock for manage devices. Selected device: ${deviceSerial}"
    List envOptions = []
    envOptions.addAll(["ROOT_DIR=${rootDir}"])
    withEnv(envOptions) {
        def result = sh(returnStatus: true,
                        label: 'book OHOS device and get connection string',
                        script: """
                            ${rootDir}/jenkins-ci/scripts/connect_hdc_device.sh -v \
                            --serial ${deviceSerial} \
                            --target-group ${targetGroup}
                        """)

        if (result != 0) {
            if (result == ERROR_FAIL_CONNECT_DEVICE) {
                String userToLock = 'OHOS_DEVICE_ERROR'
                String noteText = 'Device was removed from CI'
                // Reserve locked resource
                deviceCleanup.reserveDevice(deviceSerial, userToLock, noteText)
                String msgTitle = "Device ${deviceSerial} was removed from CI"
                String msgRecipients = ''
                withFolderProperties {
                    msgRecipients = ggApi.getEventSubscribers('device_reservation', 'huawei_id').join(',')
                }
                String issueId = ''
                try {
                    issueId = createFailDeviceIssue(deviceSerial, targetGroup)
                    if (issueId) {
                        String issueUrl = 'https://rnd-gitlab-msc.x.com/rus-os-team/devops/public/-/issues/'
                        issueId = issueUrl + issueId
                        echo "Connect device failure issue was created \n${issueId}"
                    }
                } catch (ex) {
                    echo "Error while creating a fail device issue - ${deviceSerial}\n${ex}"
                }
                String msgBody = """OHOS Device ${deviceSerial} with group ${targetGroup}
                |couldn't be connected and was removed from CI\n${issueId ?: ''}""".stripMargin()
                try {
                    sendNotification(msgTitle, msgBody, msgRecipients)
                } catch (ex) {
                    echo "Error sending notification ${msgTitle}"
                }
            }

            if (env.RETRY_ON_DEVICE_FAILURE) {
                echo '[PND] Error: Connect Device failed'
                throw new IOException('[PND] Error: Connect Device failed')
            } else {
                error('[PND] Error: Connect Device failed')
            }
        }
    }
    String varFile = "${rootDir}/connect_hdc_device.vars"
    envOptions.addAll(fileExists(varFile) ? readFile(varFile).split('\n') as List : [])
    withEnv(envOptions) {
        try {
            body.call()
            ohosDeviceCleanup(deviceSerial, targetGroup)
        } catch (ex) {
            LocalDateTime datetimeTag = LocalDateTime.now()
            echo "Final usage time for ${deviceSerial}: ${datetimeTag}"
            error("[PND] Error:\n ${ex}")
        } finally {
            // release the device on the farm
            sh "hdc-farm -v release --serial ${deviceSerial} || true"
        }
    }
}

void buildLLVMAndRunSanitizerTest(stageParams) {
    response = ggApi.getRunParams(helpers.runParams.ggRunId)
    def slurper = new groovy.json.JsonSlurper()
    def result = slurper.parseText(response)
    BZ_LLVM_PROJECT_USER_URL = result.origins[0]

    string pr_number = result.prs[0].split('/')[1]
    String rootDir = getSrcDir()
    String artifactsDir = "${rootDir}/artifacts"
    Integer flowNodeId = flowNode.getId().toInteger()
    String targetGroup = "${env.DEVICE_SOURCE_GROUP}_${env.DEVICE_TARGET_GROUP}_${currentBuild.number}_${flowNodeId}"
    Map connectParams = ['target_group': targetGroup]

    withOhosDevice(connectParams) {
        List envOptions = []
        for (p in stageParams.keySet()) {
            envOptions.add("${p}=${fillPlaceholders(stageParams[p].toString())}")
        }
        envOptions.addAll(["ROOT_DIR=${rootDir}", "PR_NUMBER=${pr_number}", "ARTIFACTS_DIR=${artifactsDir}"])
        withEnv(envOptions) {
            try {
                // download and build
                helpers.timeout_sh(stageParams,
                   "${rootDir}/jenkins-ci/scripts/build_project.sh")
                // run tests
                helpers.timeout_sh(stageParams,
                   "${rootDir}/jenkins-ci/scripts/test_project.sh")
            } catch (ex) {
                error("${ex}")
            /*} finally {
                zipFolders = stageParams.ZIP_FOLDERS ?: ['core*']
                if (!zipFolders.isEmpty()) {
                    helpers.archive("${stageParams.ARTIFACT_NAME}", rootDir, zipFolders, 'publish')
                } */               
            }
        }
    }
}
