pipeline {
    agent any

    stages {

        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        stage('Static Analysis') {
            steps {
                sh '''
                    rm -rf reports
                    mkdir -p reports

                    cppcheck \
                        --enable=all \
                        --inconclusive \
                        --check-level=exhaustive \
                        --std=c++17 \
                        --language=c++ \
                        --library=qt \
                        --library=openssl \
                        --library=windows \
                        --suppress=missingInclude \
                        --suppress=missingIncludeSystem \
                        -DQT_BEGIN_NAMESPACE= \
                        -DQT_END_NAMESPACE= \
                        --xml \
                        --xml-version=2 \
                        . \
                        2> reports/cppcheck.xml
                '''
            }
        }

        stage('Generate HTML Report') {
            steps {
                sh '''
                    mkdir -p reports/html

                    cppcheck-htmlreport \
                        --file=reports/cppcheck.xml \
                        --report-dir=reports/html \
                        --source-dir=. \
                        --title="LR1 Static Analysis"
                '''
            }
        }
    }

    post {
        always {
            archiveArtifacts artifacts: 'reports/**/*',
                             fingerprint: true
        }
    }
}
