#! /bin/bash
#################################################################################
#     File Name           :     test.sh
#     Created By          :     sil@andrew.cmu.edu
#     Creation Date       :     [2016-10-30 17:57]
#     Last Modified       :     [2016-10-30 18:12]
#     Description         :
#################################################################################
sudo umount /coda && sudo killall venus
make
sudo make install
sudo rm -rf /var/lib/coda
sudo coda-client-setup new
sudo venus setup


