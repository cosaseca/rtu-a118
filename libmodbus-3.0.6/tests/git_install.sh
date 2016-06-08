sudo apt-get install git-core openssh-server openssh-client
git config --global user.name "ygz"
git config --global user.email "ygz012@163.com"
sudo apt-get install python-setuptools
sudo groupadd apache
sudo useradd --system --home-dir /home/git -c 'git version control' -G apache --shell /bin/sh -U -m git
cd /tmp
git clone https://github.com/res0nat0r/gitosis.git
cd gitosis
sudo python setup.py install

sudo cp ~/.ssh/id_rsa.pub /tmp/id_rsa.pub
sudo chmod a+r /tmp/id_rsa.pub

sudo -H -u git gitosis-init < /tmp/id_rsa.pub

sudo chmod 755 /home/git/repositories/gitosis-admin.git/hooks/post-update
