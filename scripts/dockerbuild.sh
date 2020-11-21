set -e

# trap not available in /sh .. darn
# keep track of the last executed command
# trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
# echo an error message before exiting
# trap 'echo "\"${last_command}\" command filed with exit code $?."' EXIT

# install the dependencies
apk update && apk upgrade && apk add curl bash git openssh

# pulling the image. default to master if branch doesn't have an image
if [ $(curl --silent https://gitlab-student.centralesupelec.fr/api/v4/groups/$CI_PROJECT_NAMESPACE/registry/repositories?tags=1 | grep $CI_COMMIT_BRANCH) ]; then
    IMAGE_BRANCH=$CI_COMMIT_BRANCH
else
    IMAGE_BRANCH=$CI_DEFAULT_BRANCH
fi 

echo "Using image: $CI_REGISTRY/$CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$IMAGE_BRANCH"
docker pull gitlab-student.centralesupelec.fr:4567/$CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$IMAGE_BRANCH

echo "changed file: $(git diff --name-only $CI_COMMIT_BEFORE_SHA $CI_COMMIT_SHA) "
echo detected files: $(git diff --name-only $CI_COMMIT_BEFORE_SHA $CI_COMMIT_SHA -- Dockerfile .gitlab-ci.yml requirements.txt)

# build the image, or use the existing one if no changes
if [ $(git diff --quiet --name-only $CI_COMMIT_BEFORE_SHA $CI_COMMIT_SHA -- Dockerfile .gitlab-ci.yml requirements.txt) ]; then
    echo "One or more of Dockerfile, requirements.txt and .gitlab-ci.yml changed. Building"
    docker build -t gitlab-student.centralesupelec.fr:4567/$CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$CI_COMMIT_BRANCH --cache-from $CI_REGISTRY/$CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$IMAGE_BRANCH . > docker-build.log
else
    echo "Adding tag to default branch:"
    docker tag gitlab-student.centralesupelec.fr:4567/$CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$IMAGE_BRANCH gitlab-student.centralesupelec.fr:4567/$CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$CI_COMMIT_BRANCH 
fi

# pushing the image
echo "Pushing image: "
docker push gitlab-student.centralesupelec.fr:4567/$CI_REGISTRY_USER/$CI_REGISTRY_IMAGE:$CI_COMMIT_BRANCH
