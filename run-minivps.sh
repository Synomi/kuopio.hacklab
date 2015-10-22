#!/bin/bash
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color
echo "Kuopio Hacklab ry, docker container creator for 'minivps' stuff.."
echo -e "Please give member name(used as subdomain) [a-z,0-9]:\c"
read name
echo -e "Please give mysql root password:\c"
read mysql_password
echo -e "Please give ssh port (check excel for free port):\c"
read ssh_port

printf "This will create ${GREEN}$name-mysql${NC} named mysql-container with root password: $mysql_password. Which can be accessed from $name-nginx and $name-minivps.\n"
printf "This will create ${GREEN}$name-nginx${NC} named nginx-php-fpm container which links to the mysql container and can be accessed from http://$name.idim.space.\n"
printf "This will create ${GREEN}$name-minivps${NC} named modified ssh-base container which links to the mysql container and gets volumes from $name-nginx container.\n"
printf "${GREEN}$name-minivps${NC} container can be accessed by 'ssh root@$name.idim.space -p$ssh_port with password: kuopiohacklab2015, ${RED}change password${NC} with ${GREEN}passwd${NC}!.\n"
printf "First containers with above names will be stopped&removed and then recreated. So any previous data with above names will be ${RED}deleted${NC}.\n"
echo "Do you wish to continue? (y/n)"
read continue
if [ $continue == y ]
then
        echo "Stopping, removing and recreating the containers. Do not mind errors if containers do not exist yet ..."
else
        exit
fi

#poista mahdolliset aiemmat containerit
docker stop ${name}-mysql ${name}-minivps ${name}-nginx
docker rm ${name}-mysql ${name}-minivps ${name}-nginx

#https://hub.docker.com/_/mysql/
docker run --name ${name}-mysql -e MYSQL_ROOT_PASSWORD=${mysql_password} -d mysql

#https://hub.docker.com/r/richarvey/nginx-php-fpm/ link mysql
docker run --name ${name}-nginx --link ${name}-mysql:mysql -e VIRTUAL_HOST=${name}.idim.space -d richarvey/nginx-php-fpm

#https://hub.docker.com/r/synomi/base-ssh-server/ link mysql, volumes from nginx
docker run --name ${name}-minivps --link ${name}-mysql:mysql --volumes-from ${name}-nginx -p ${ssh_port}:22 -d synomi/base-ssh-server
docker exec ${name}-minivps bash -c 'echo $MYSQL_PORT > mysql_port.txt'













