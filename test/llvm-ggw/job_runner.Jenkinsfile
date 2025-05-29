/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
echo 'BZ PRE MERGE LLVM'
String libBranchName = params?.ark_ci_revision ?: 'master'
def lib = library("llvm_ci_helpers@${libBranchName}").com.x.rnd.devops
def stageStatus = lib.StagesStatus.new()

// Init helpers
echo 'initDepTree'
helpers.initDepTree()
echo 'initRepoManifest'
helpers.initRepoManifest()
echo 'initRunParams'
helpers.initRunParams()

env.REPO_TYPE = 'repo_emui'
env.NEXUS_REPO = 'nexus-cn-yz.devops-spb.rnd.x.com'
env.NEXUS_PROTOCOL = 'https'

def errorException = 'none'
String ciStatus = 'passed'

env.PIPELINE_NAME = 'LLVM'

// Init stage
// echo 'runPreMergeInitStage'
// pipeStages.runPreMergeInitStage()

//We define default image here but we need to change it for each job
echo 'getImage'
String image = helpers.getImage()
Map cloud = ['cloud' : 'ark-cloud', 'template' : ['panda-1cpu-proxy', 'panda-1cpu'], 'image' : image]

echo 'Prepare'
try {
    config = libraryResource('gitlab_wrapper.yml')
    Map<String, Map> job = (Map<String, Map>) readYaml(text: config)
    echo 'Config ready'

    // Get the first job from yaml. It is assumed that there is only one
    revJobName = job.keySet().find { true }
    // Save checkReposRegexFilter to the job params
    job[revJobName].checkReposRegexFilter = []
    echo "JOB: ${revJobName}"

    // Generate and run the stage
    stg = helpers.prepareOneStage(cloud, revJobName, job[revJobName], stageStatus)
    stg.call()
} catch (ex) {
    ciStatus = 'failed'
    errorException = ex
    echo "Failed: ${ex}"
}

if (helpers.wasPipelineCanceled(errorException)) {
    ciStatus = 'cancelled'
}

pipeStages.runSendResultStage(ciStatus)

helpers.processCiStatus(ciStatus, errorException)
