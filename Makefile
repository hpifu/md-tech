repository=$(shell basename -s .git $(shell git config --get remote.origin.url))
user=hatlonely
version=$(shell git describe --tags)

.PHONY: image
image:
	docker build --tag=hatlonely/${repository}:${version} .
	cat stack.tpl.yml | sed 's/\$${version}/${version}/g' | sed 's/\$${repository}/${repository}/g' > stack.yml


.PHONY: deploy
deploy:
	# docker network create --driver overlay --attachable attachable-overlay-network
	# docker service update --network-rm attachable-overlay-network mysqlnet
	# docker service update --network-add attachable-overlay-network mysql_mysql
	# docker run -it --network=attachable-overlay-network hatlonely/${repository}:${version}
	docker jobs create --network=mysqlnet hatlonely/${repository}:${version}
	# docker stack deploy -c stack.yml ${repository}

.PHONY: remove
remove:
	docker stack rm ${repository}

dist:
	npm run build