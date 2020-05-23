for i in $(cat swarmpeers.txt); do
    ipfs swarm connect $i
done
