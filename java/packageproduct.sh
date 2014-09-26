mvn clean package -P product -Dmaven.test.skip=true
mvn dependency:copy-dependencies
