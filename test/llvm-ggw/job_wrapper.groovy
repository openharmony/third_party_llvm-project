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


/* groovylint-disable-next-line UnusedMethodParameter */
void launchLLVMGitlabCI(stageParams) {
    echo "stageParams ${stageParams}"

    response = ggApi.getRunParams(helpers.runParams.ggRunId)
    echo "Response: ${response}"
    def slurper = new groovy.json.JsonSlurper()
    def result = slurper.parseText(response)

    BZ_LLVM_PROJECT_USER_URL = result.origins[0]
    echo "BZ_LLVM_PROJECT_USER_URL = ${BZ_LLVM_PROJECT_USER_URL}"
    PR_NUMBER = result.prs[0].split('/')[1]
    echo "PR_NUMBER = ${PR_NUMBER}"
    if (BZ_LLVM_PROJECT_USER_URL.endsWith(".git")) {
        BZ_LLVM_PROJECT_USER_URL = BZ_LLVM_PROJECT_USER_URL.replace(".git", "")
        echo "changed BZ_LLVM_PROJECT_USER_URL = ${BZ_LLVM_PROJECT_USER_URL}"
    }
    BZ_MUSL_USER_URL = ""
    TARGET_FILTER = "ohos"
    REF_BRANCH = "x/blue-zone-open-harmonyos"
    ADD_BUILD_PARAMS = "--no-build-x86_64 --no-build-mipsel --no-build-arm --no-build-riscv64 --no-build windows,lldb-server,check-api"
    TARGET_DEVICE = "hdc-device"
    TARGET_TRIPLE = "aarch64-linux-ohos"
    DEVLIB_TARGET_TYPE = "OHOS"
    DEVLIB_CONN_TYPE = "HDC"
    HDC_FARM_URL = "https://ark-hdc-farm-yz.devops-spb.rnd.x.com"
    HDC_DEVICE_CI_LABEL = "LLVM-CI"
    HDC_DEVICE_ARCH_LABEL = "AARCH64"

    if (result.custom[0]) {
        SUBSET_FILTER = result.custom[0]
    } else {
        SUBSET_FILTER = "toolchain_simple"
    }

    if (!SUBSET_FILTER.contains("toolchain")) {
        SUBSET_FILTER = "toolchain_simple"
    }

    echo "subset from gitee ${SUBSET_FILTER}"
    def file_diff_content = sh(returnStdout: true, script: """curl https://gitee.com/openharmony/third_party_llvm-project/pulls/${PR_NUMBER}.diff""")
    LLDB_CHANGED = file_diff_content.contains("+++ b/lldb")

    echo "LLDB_CHANGED = ${LLDB_CHANGED}"

    if (SUBSET_FILTER == 'toolchain_debugger' || LLDB_CHANGED) {
        ADD_BUILD_PARAMS = "--no-build-x86_64 --no-build-mipsel --no-build-arm --no-build-riscv64 --no-build windows"
    }
    withCredentials([string(credentialsId: 'LLVM_CI_TOKEN', variable: 'LLVM_CI_TOKEN'),
         string(credentialsId: 'LLVM_ACCESS_TOKEN', variable: 'LLVM_ACCESS_TOKEN')
         ]) {
        try {

            response = sh(returnStdout: true, script: """curl -X POST \
                             --fail \
                             -F token=${LLVM_CI_TOKEN} \
                             -F ref=${REF_BRANCH} \
                             -F variables[BZ_LLVM_PROJECT_USER_URL]=${BZ_LLVM_PROJECT_USER_URL} \
                             -F variables[BZ_MUSL_USER_URL]=${BZ_MUSL_USER_URL} \
                             -F variables[TOOLCHAIN_MANIFEST_NAME]=llvm-toolchain.xml \
                             -F variables[MANIFEST_REPOSITORY_URL]=https://gitee.com/openharmony/manifest.git \
                             -F variables[TOOLCHAIN_MANIFEST]=master \
                             -F variables[TARGET_FILTER]=${TARGET_FILTER} \
                             -F variables[BUILD_FAST]=true \
                             -F variables[SUBSET_FILTER]=${SUBSET_FILTER} \
                             -F variables[ADD_BUILD_PARAMS]="${ADD_BUILD_PARAMS}" \
                             -F variables[TARGET_DEVICE]="${TARGET_DEVICE}" \
                             -F variables[TARGET_TRIPLE]="${TARGET_TRIPLE}" \
                             -F variables[DEVLIB_TARGET_TYPE]="${DEVLIB_TARGET_TYPE}" \
                             -F variables[DEVLIB_CONN_TYPE]="${DEVLIB_CONN_TYPE}" \
                             -F variables[HDC_FARM_URL]="${HDC_FARM_URL}" \
                             -F variables[HDC_DEVICE_CI_LABEL]="${HDC_DEVICE_CI_LABEL}" \
                             -F variables[HDC_DEVICE_ARCH_LABEL]="${HDC_DEVICE_ARCH_LABEL}" \
                             https://rnd-gitlab-msc.x.com/api/v4/projects/3091/trigger/pipeline
            """)
            sh(label: 'Install jq', script: 'apt-get update && apt-get install --no-install-recommends -y jq')

            JSONObject respJson = readJSON(text: response)
            pipeId = respJson['id']
            echo "Triggered ${respJson['web_url']}"
            sh(label: 'Detailed gitlab response:', script: "echo ${response}")
            sh(label: 'Wait GitLab CI to finish', script: """while true; do \
                  sleep 20; \
                  curl --header 'PRIVATE-TOKEN: ${LLVM_ACCESS_TOKEN}' \
                  https://rnd-gitlab-msc.x.com/api/v4/projects/3091/pipelines/${pipeId} 2>/dev/null | \
                  jq .finished_at | \
                  grep null || break; \
                done
            """)
        } catch (FlowInterruptedException ex) {
            echo "in FlowInterruptedException"
            response = sh(returnStdout: true, label: 'Get GitLab CI cancel', script:
            """curl --request POST --header 'PRIVATE-TOKEN: ${LLVM_ACCESS_TOKEN}' \
                    https://rnd-gitlab-msc.x.com/api/v4/projects/3091/pipelines/${pipeId}/cancel
            """)
            respJson = readJSON(text: response)
            sh(label: "status: ${respJson['status']}", script: "echo ${response}")
        } finally {
            response = sh(returnStdout: true, label: 'Get GitLab CI result', script:
            """curl --header 'PRIVATE-TOKEN: ${LLVM_ACCESS_TOKEN}' \
                    https://rnd-gitlab-msc.x.com/api/v4/projects/3091/pipelines/${pipeId}
            """)
            respJson = readJSON(text: response)
            sh(label: "status: ${respJson['status']}", script: "echo ${response}")
            if (respJson['status'] != 'success') {
                error("GitLab CI failed: ${respJson['web_url']}/failures")
            }
        }
    }
}
