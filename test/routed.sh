echo "------------------ Running test $1 -------------------"
if [[ ! -f zenohd ]]; then
  if [[ "$OSTYPE" == "darwin"* ]]; then 
    echo "> Downloading https://github.com/atolab/atobin/raw/master/zenoh/latest/macos/10.15.5/zenohd ..."
    curl -L -o zenohd https://github.com/atolab/atobin/raw/master/zenoh/latest/macos/10.15.5/zenohd
  elif [[ "$OSTYPE" == "linux-gnu" ]] && [[ -f /etc/redhat-release ]]; then 
    echo "> Downloading https://github.com/atolab/atobin/raw/master/zenoh/latest/centos/7.2.1511/zenohd ..."
    curl -L -o zenohd https://github.com/atolab/atobin/raw/master/zenoh/latest/centos/7.2.1511/zenohd
  else 
    echo "> Downloading https://github.com/atolab/atobin/raw/master/zenoh/latest/ubuntu/16.04/zenohd ..."
    curl -L -o zenohd https://github.com/atolab/atobin/raw/master/zenoh/latest/ubuntu/16.04/zenohd
  fi
fi
chmod +x zenohd

echo "> Running zenohd ..."
./zenohd --verbosity=debug > zenohd.$1.log &
ZPID=$!
sleep 0.2

echo "> Running $1 ..."
./$1
RETCODE=$?

echo "> Stopping zenohd ..."
kill -9 $ZPID

echo "> Done ($RETCODE)."
exit $RETCODE