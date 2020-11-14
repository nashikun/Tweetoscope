apk update && apk upgrade && apk add curl bash git openssh

if [ $(curl --silent -f -lSL https://hub.docker.com/v2/repositories/$CI_REGISTRY_USER/$CI_REGISTRY_IMAGE/tags/$CI_COMMIT_BRANCH 2>/dev/null) ]; then
    IMAGE_BRANCH=$CI_COMMIT_BRANCH
else
    IMAGE_BRANCH=$CI_DEFAULT_BRANCH
fi 

docker pull $CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$IMAGE_BRANCH
git clone --depth 1 --no-single-branch https://gitlab-ci-token:${CI_JOB_TOKEN}@gitlab-student.centralesupelec.fr:tweetoscope-mullticolore/tweetou.git

if [ $(git --git-dir tweetou/.git diff origin/$CI_DEFAULT_BRANCH origin/CI_COMMIT_BRANCH --name-only | grep -e Dockerfile -e requirements.txt -e .gitlab-ci.yml) ]; then
    docker build -t $CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$CI_COMMIT_BRANCH --cache-from $CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$IMAGE_BRANCH . > docker-build.log
else
    docker tag $CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$IMAGE_BRANCH $CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$CI_COMMIT_BRANCH 
fi

docker push $CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$CI_COMMIT_BRANCH
