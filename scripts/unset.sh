#!/bin/sh
iptables -t filter -D FORWARD  -i br-lan -p tcp --dport 80 -m state --state ESTABLISHED -j info_chain
iptables -t filter -F info_chain
iptables -t filter -X info_chain

