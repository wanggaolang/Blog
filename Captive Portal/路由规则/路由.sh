IPTABLES=/sbin/iptables

#设置一个ip集合存储已通过验证ip，自动重新验证时间为12h
ipset create goodip hash:ip timeout 43200

#制作一个新的链，放入mangle表
$IPTABLES -N internet -t mangle

#将所有收到的数据包放入internet链筛选
$IPTABLES -t mangle -A PREROUTING -j internet

#将INTERNET链中的包的源地址与已登陆ip集合goodip比对，若存在则跳出此链
iptables -A internet -m set --match-set goodip src  -j RETURN

#将剩下的（即未放入允许登陆集goodip）标记为49
$IPTABLES -t mangle -A internet -j MARK --set-mark 49

#将标记位49的包目标地址转换为认证页面
$IPTABLES -t nat -A PREROUTING -m mark --mark 49 -j DNAT --to-destination 10.233.233.254：8080

#将标记为49的包丢弃，由于在PREROUTING没有filter表，所以这里是最早能够丢弃的地方
$IPTABLES -t filter -A FORWARD -m mark --mark 49 -j DROP

#为了防止未登录用户走INPUT链通过Squid获取信息，对他们只开放80和53端口（好狠！）
$IPTABLES -t filter -A INPUT -p tcp --dport 8080 -j ACCEPT
$IPTABLES -t filter -A INPUT -p udp --dport 53 -j ACCEPT
$IPTABLES -t filter -A INPUT -m mark --mark 49 -j DROP

# 通过验证的包进行转发（联网），第一行是开启内核的IP_FORWARD功能
#第四行是源地址ip转换，MASQUERADE相当于SNAT的升级版
echo "1" > /proc/sys/net/ipv4/ip_forward
$IPTABLES -A FORWARD -i ppp0 -o eth0 -m state --state ESTABLISHED,RELATED -j ACCEPT
$IPTABLES -A FORWARD -i eth0 -o ppp0 -j ACCEPT
$IPTABLES -t nat -A POSTROUTING -o ppp0 -j MASQUERADE