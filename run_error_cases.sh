# Loop through each error case 1-16 and output the results to a file

for i in {1..16}
do
    echo "Running error case $i"
    ./a.out error$i.txt error$i\out.txt
done