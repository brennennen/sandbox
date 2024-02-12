


These can be ran in another container or on the host os.

Kafka expects the brokers to be added to the hosts file. To not cludge up
my hosts file, I've included the `deploy_local.bash` to spin up
a temporary container to configure and run scripts on.