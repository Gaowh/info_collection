#!/bin/sh

iptables -t filter -N info_chain
iptables -t filter -I FORWARD  -i br-lan -p tcp --dport 80 -m state --state ESTABLISHED -j info_chain

source /bh_manage/info_collection/config

if [ "$CONFIG_m_jd" == "y" ] ; then
	#mobile jd 京东的客户端和网页搜索时不同，这里定义了两条规则
	iptables -t filter -A info_chain -m string --string "Host: search.m.jd.com" --algo kmp\
		 -j ULOG --ulog-nlgroup 12 --ulog-prefix "12" --ulog-cprange 1024

	iptables -t filter -A info_chain -m string --string "Host: m.jd.com" --algo kmp\
		-j ULOG --ulog-nlgroup 12 --ulog-prefix "12" --ulog-cprange 1024
fi

if [ "$CONFIG_m_tm" == "y" ] ; then
	#mobile tm
	iptables -t filter -A info_chain -m string --string "Host: suggest.taobao.com" --algo kmp\
		 -j ULOG --ulog-nlgroup 12 --ulog-prefix "22" --ulog-cprang 1024
fi


if [ "$CONFIG_m_sn" == "y" ] ; then
	#mobile sn
	iptables -t filter -A info_chain -m string --string "Host: search.suning.com" --algo kmp \
	--from 0 --to 350  -j ULOG --ulog-nlgroup 12 --ulog-prefix "42" --ulog-cprange 1024
fi

if [ "$CONFIG_m_pp" == "y" ]; then
	#mobile pp
	iptables -t filter -A info_chain -m string --string "Host: search.paipai.com" --algo kmp\
		--from 0 --to 350 -j ULOG --ulog-nlgroup 12 --ulog-prefix "52" --ulog-cprange 1024

fi

if [ "$CONFIG_m_dd" == "y" ]; then
	#mobile dd
	iptables -t filter -A info_chain -m string --string "Host: mapi.dangdang.com" --algo kmp\
		-j ULOG --ulog-nlgroup 12 --ulog-prefix "62"  --ulog-cprange 1024
fi

if [ "$CONFIG_m_yx" == "y" ] ; then
	#mobile yx
	iptables -t filter -A info_chain -m string --string "Host: mb.51buy.com" --algo kmp\
		 -j ULOG --ulog-nlgroup 12 --ulog-prefix "72" --ulog-cprang 1024
fi

if [ "$CONFIG_m_yhd" == "y" ] ; then
	#mobile yhd 一号店的客户端和网页搜索时不同，定义了两条规则
	iptables -t filter -A info_chain -m string --string "Host: mapi.yhd.com" --algo kmp\
		-j ULOG --ulog-nlgroup 12 --ulog-prefix "82"  --ulog-cprange 1024
	
	iptables -t filter -A info_chain -m string --string "Host: sezrch.m.yhd.com" --algo kmp\
		-j ULOG --ulog-nlgroup 12 --ulog-prefix "82"  --ulog-cprange 1024

fi

if [ "$CONFIG_m_jmyp" == "y" ] ; then
	#mobile jmyp 
	iptables -t filter -A info_chain -m string --string "Host: s.mobile.jumei.com" --algo kmp\
		-j ULOG --ulog-nlgroup 12 --ulog-prefix "92"  --ulog-cprange 1024
fi


###########################################################################################

#jd

if [ "$CONFIG_jd" == "y" ]; then
iptables -t filter -A info_chain  -m string --string "Host: search.jd.com" --algo kmp \
	--from 0 --to 290 -j ULOG --ulog-nlgroup 12 --ulog-prefix "11" --ulog-cprange 1024

fi

if [ "$CONFIG_tm" == "y" ] ; then
#tm
iptables -t filter -A info_chain  -m string --string "Host: list.tmall.com" --algo kmp \
	--from 0 --to 300 -j ULOG --ulog-nlgroup 12 --ulog-prefix "21" --ulog-cprange 400

fi


if [ "$CONFIG_tb" == "y" ]; then
#tb
iptables -t filter -A info_chain -m string --string "Host: s.taobao.com" --algo kmp \
	--from 0 --to 380 -j ULOG --ulog-nlgroup 12 --ulog-prefix "31" --ulog-cprange 500

fi

if [ "$CONFIG_pp" == "y" ] ;then
#pp
iptables -t filter -A info_chain  -m string --string "Host: se.paipai.com" --algo kmp \
	--from 0 --to 300 -j ULOG --ulog-nlgroup 12 --ulog-prefix "51" --ulog-cprange 400


fi

if [ "$CONFIG_dd" == "y" ] ; then
#dd
iptables -t filter -A info_chain  -m string --string "Host: search.dangdang.com" --algo kmp \
	--from 0 --to 300 -j ULOG --ulog-nlgroup 12 --ulog-prefix "61" --ulog-cprange 400 	
fi

if [ "$CONFIG_yx" == "y" ] ; then
#yx
iptables -t filter -A info_chain -m string --string "Host: searchex.yixun.com" --algo kmp\
	--from 0 --to 250 -j ULOG --ulog-nlgroup 12 --ulog-prefix "71" --ulog-cprange 200

fi

iptables -t filter -A info_chain -j RETURN 
