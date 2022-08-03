#!/bin/bash
CURRENT_DIR=$PWD

while true; do
    read -p "Remove whitespace on all PDF files in directory $1? ... " yn
    case $yn in
        [Yy]* ) cd $1; break;;
        [Nn]* ) exit;;
        * ) echo "Please answer yes or no.";;
    esac
done


for i in *.pdf; do
    [ -f "$i" ] || break
    pdfcrop --margins="0 0 50 0" $i $i
done

cd $CURRENT_DIR
