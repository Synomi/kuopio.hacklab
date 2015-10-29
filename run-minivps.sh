#!/bin/bash
genpasswd() {
        local l=$1
        [ "$l" == "" ] && l=8
        tr -dc A-Za-z0-9_ < /dev/urandom | head -c ${l} | xargs
}
generated_password=$(genpasswd 8)
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color
echo "Kuopio Hacklab ry, docker container creator for 'minivps' stuff.."
echo -e "Please give member name(used as subdomain) [a-z,0-9]:\c"
read name
echo -e "Please give member id-100 (used as ssh-port) [80xx]:\c"
read ssh_port

printf "This will create ${GREEN}$name-nginx${NC} named nginx-php-fpm container which links to shared-mysql container and can be accessed from http://$name.idim.space.\n"
printf "This will create ${GREEN}$name-minivps${NC} named modified ssh-base container which links to shared-mysql container and gets volumes from $name-nginx container.\n"
printf "${GREEN}$name-minivps${NC} container can be accessed by 'ssh root@$name.idim.space -p$ssh_port with password: ${GREEN}$generated_password${NC}, ${RED}change password${NC} with ${GREEN}passwd${NC}!.\n"
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
docker stop ${name}-minivps ${name}-nginx
docker rm ${name}-minivps ${name}-nginx

#https://hub.docker.com/r/richarvey/nginx-php-fpm/ link mysql
docker run --name ${name}-nginx --link minivps-shared-mysql:mysql -e VIRTUAL_HOST=${name}.idim.space -d richarvey/nginx-php-fpm

#https://hub.docker.com/r/synomi/base-ssh-server/ link mysql, volumes from nginx
docker run --name ${name}-minivps --link minivps-shared-mysql:mysql --volumes-from ${name}-nginx -p ${ssh_port}:22 -d synomi/base-ssh-server
docker exec ${name}-minivps bash -c 'echo $MYSQL_PORT > mysql_port.txt'

#change root pass to: $generated_password
passwordString="$generated_password\n$generated_password"
passwordCommand="echo -e '$passwordString' | passwd"
docker exec -it $name-minivps bash -c "$passwordCommand"

#mysql drop user&database
mysqlDropUserCmd="DROP USER ${name}"
mysqlDropDatabaseCmd="DROP DATABASE ${name}"
docker exec minivps-shared-mysql bash -c "mysql -uroot -pxxx -e '$mysqlDropUserCmd'"
docker exec minivps-shared-mysql bash -c "mysql -uroot -pxxx -e '$mysqlDropDatabaseCmd'"

#mysql create user&database
mysqlCreateDatabaseCmd="CREATE DATABASE ${name}"
mysqlCreateUserCmd="GRANT ALL PRIVILEGES ON ${name}.* TO ${name} IDENTIFIED BY \"${generated_password}\""
mysqlCreateUserFullCmd="mysql -uroot -pxxx -e '${mysqlCreateUserCmd}'"
docker exec minivps-shared-mysql bash -c "mysql -uroot -pxxx -e '$mysqlCreateDatabaseCmd'"
docker exec minivps-shared-mysql bash -c "$mysqlCreateUserFullCmd"

