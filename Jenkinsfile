pipeline {
    agent any
    stages {
        stage('image') {
            steps {
                sh 'make image'
            }
        }
        stage('deploy') {
            steps {
                sh 'make deploy'
            }
        }
    }
}
