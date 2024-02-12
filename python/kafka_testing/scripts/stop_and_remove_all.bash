docker stop $(docker ps -aq)
docker rm --force $(docker ps -aq)
#docker rmi --force $(docker images -q)