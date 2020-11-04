apk update && apk add curl

if [ $(curl --silent -f -lSL https://hub.docker.com/v2/repositories/$CI_REGISTRY_USER/$CI_REGISTRY_IMAGE/tags/$CI_COMMIT_BRANCH 2>/dev/null) ]; then
    docker pull $CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$CI_COMMIT_BRANCH
fi 

docker pull $CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$CI_DEFAULT_BRANCH

if [ $(git diff master --name-only | grep -e requirements.txt -e .gitlab-ci.yml) ]; then
    docker build -t $CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$CI_COMMIT_BRANCH --cache-from $CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$CI_COMMIT_BRANCH --cache-from $CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$CI_DEFAULT_BRANCH . > docker-build.log
else
    docker tag $CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$CI_DEFAULT_BRANCH $CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$CI_COMMIT_BRANCH 
fi

docker push $CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$CI_COMMIT_BRANCH
