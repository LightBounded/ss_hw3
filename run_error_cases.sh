# Pass each file prefixed with error to the compiler and output the error to a file prefixed with error*out.txt

for file in error*.txt
do
    echo "Running $file"
    ./a.out $file ${file%.txt}out.txt
done