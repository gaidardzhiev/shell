printf "\ntesting conditionals with [ ... ]\n"
[ 16 -gt 8 ] && echo "16 is greater than 8"
[ 8 -gt 18 ] || echo "8 is not greater than 16"
[ -f /etc/passwd ] && echo "/etc/passwd exists"
[ -d /tmp ] && echo "/tmp is a directory"

printf "\ntesting chaining\n"
false || echo "previous failed"
true && echo "this runs after success"

printf "\ntesting variables\n"
export COUNT=15
[ -n "$COUNT" ] && echo "COUNT is set"
[ "$COUNT" -gt 10 ] && echo "COUNT is greater than 10"
[ "$COUNT" = "15" ] && echo "COUNT equals 15"
[ -z "$EMPTYVAR" ] && echo "EMPTYVAR is empty or not set"


printf "\ntesting redirection\n"
echo hello > test.txt
cat test.txt && rm test.txt
printf "\nend of test\n"
