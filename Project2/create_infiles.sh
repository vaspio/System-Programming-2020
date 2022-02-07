#!/bin/bash
echo ""

if [ $# -ne 5 ]; then   # todo: check for positives
    echo "Wrong input"  # and last 2 only are numbers
    exit 1
fi

echo "$1 $2 $3 $4 $5"

countries=$1
diseases=$2
in_dir=$3
num_files=$4
num_records=$5
id=0

mkdir -p $in_dir

while read country
do
    readarray -t dis_array < $diseases # Read into array
    num_disease=${#dis_array[*]}     # Count elements

    cd $in_dir

    mkdir -p $country
    cd $country

    for((fl=1; fl <= $num_files ;fl++))
    do
        dd=$(( ( RANDOM % 30 )  + 1 ))
        mm=$(( ( RANDOM % 12 )  + 1 ))
        yy=$(( ( RANDOM % 15 )  + 2005 ))
        # echo "$dd-$mm-$yy"
        datefile="$dd-$mm-$yy"

        touch $datefile
        # mkdir -p "$dd-$mm-$yy"

        for((rec=1; rec <= $num_records ;rec++))
        do
            id=$((id+1))

            enex=$((RANDOM % 2))
            if [ $enex == 0 ]; then
                enex='ENTER'
            else
                enex='EXIT'
            fi

            ran=$(((RANDOM % 10) + 2))  #random string names(3-12 char)
            f=$(shuf -er -n1 {A..Z})    #first letter uppercase
            first=$(head /dev/urandom | tr -dc a-z | head -c $ran)

            ran=$(((RANDOM % 10) + 2))
            l=$(shuf -er -n1 {A..Z})
            last=$(head /dev/urandom | tr -dc a-z | head -c $ran)

            disease_name=${dis_array[$((RANDOM%num_disease))]}
            age=$(((RANDOM % 120) + 1))

            echo "$id $enex $f$first $l$last $disease_name $age" >> $datefile
        done
    done

    cd ../..

done < $countries
