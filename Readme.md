# ScyllaDB Assignment

# Notes:
- sources of seastar are copied and provided together with this repository since it contains reference to not-existing FindProtobuf which I had to remove to be able to install it inside the docker
- I've used part of the procided docker file which uses ubuntu:mantic as a base image
- I'm not using seastar logger as it was causing linkage errors `undefined reference to `seastar::logger::failed_to_log` and I didn't want to spent more time on investigation