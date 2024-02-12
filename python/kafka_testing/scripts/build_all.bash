#!/usr/bin/env bash
cd $(dirname $BASH_SOURCE)
bash ./../components/zookeeper/build.bash
bash ./../components/broker/build.bash
bash ./../components/producer/build.bash
bash ./../components/consumer/build.bash
cd -