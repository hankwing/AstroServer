#/bin/bash

rm -rf ../template_table/*
echo "delete $(hostname)'s template table."
username=$(whoami)
for line in `cat nodehostname`
do
{
ssh $username@$line "rm -rf gwac/template_table/*"
echo "delete $line's template table"
} &

done
wait
