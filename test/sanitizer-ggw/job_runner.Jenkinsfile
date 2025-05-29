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

String libBranchName = params?.ark_ci_revision ?: 'master'
def lib = library("llvm_ci_helpers@${libBranchName}").com.x.rnd.devops
def jobFilter = lib.JobFilter.of('dummy') { }
def stageStatus = lib.StagesStatus.new()

// Init helpers
helpers.initDepTree()
helpers.initRepoManifest()
helpers.initRunParams()

env.REPO_TYPE = 'repo_emui'

def errorException = 'none'
String ciStatus = 'passed'

env.PIPELINE_NAME = 'LLVM_ASAN'
withFolderProperties {
    env.MAX_STAGE_TIMEOUT = env.PRE_MERGE_ARKJS_MAX_STAGE_TIMEOT?.toInteger() ?: 240
    env.DEFAULT_PAYLOAD_TIMEOUT = env.PRE_MERGE_ARKJS_PAYLOAD_TIMEOUT?.toInteger() ?: 220
    env.DEFAULT_RETRIES_IN_STRICT_MODE = env.PRE_MERGE_ARKJS_DEFAULT_RETRIES_IN_STRICT_MODE?.toInteger() ?: 1
}

// Init stage
pipeStages.runPreMergeInitStage()

//We define default image here but we need to change it for each job
String image = helpers.getImage()
Map cloud = ['cloud' : 'helium-cloud', 'template' : ['panda-1cpu-proxy', 'panda-1cpu'], 'image' : image]
int timeout = env.DEFAULT_STAGE_TIMEOUT.toInteger()
Map props = ['retries': 1, 'timeout': timeout]

void runStage(String name, List configFiles, Map cloud, stageStatus, List filters = []) {
    stage(name) {
        if (helpers.runParams.isShuttle()) {
            // Clear the shuttle archive name for the second stage because them should use re-packed tar-ball
            // with ark standalone changes after the first stage' job
            helpers.repoManifest.archiveName = ''
        }
        List configs = configFiles.collect { c -> libraryResource(c) }
        buildParallelMap = helpers.prepareParallelStage(cloud, configs, stageStatus, [:], filters)
        buildParallelMap.put('failFast', env.RTM?.toBoolean())
        parallel(buildParallelMap)
    }
}

try {
    // Need to clone repos before checking if the pipe is required
//    if (!helpers.runParams.isShuttle()) {
        pipeStages.runInitRevisionStage(cloud, props, stageStatus)
//    }

    List configs = []

    List filters = []
//    List filters = jobFilters.filters(libraryResource('job_filters.yml'))
/*    stage('Get sources standalone') {
        configs = [libraryResource('arkjsvm/get_sources_standalone.yml')]
        Map buildParallelMap = helpers.prepareParallelStage(cloud, configs, stageStatus, [:], filters)
        buildParallelMap.put('failFast', env.RTM?.toBoolean())
        parallel(buildParallelMap)
    }
*/
    buildConfigs = ['llvm_asan_device.yml']
    runStage('Build', buildConfigs, cloud, stageStatus, filters)

//    runStage('Test', arkjsHelpers.testStageConfigFiles(), cloud, stageStatus, filters)
} catch (ex) {
    ciStatus = 'failed'
    errorException = ex
}

if (helpers.wasPipelineCanceled(errorException)) {
    ciStatus = 'cancelled'
}

pipeStages.runSendResultStage(ciStatus)

helpers.processCiStatus(ciStatus, errorException)
