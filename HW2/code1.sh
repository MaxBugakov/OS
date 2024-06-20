echo "Enter a:"
read a
echo "Enter b:"
read b
if [ $a -gt $b ]; then
	echo "a is greater than b"
elif [ $a -eq $b ]; then 
	echo "a equals b"
else
	echo "a is less than b"
fi
