printf "testing one-line conditionals with [ ... ]\n"
[ 16 -gt 8 ] && echo "16 is greater than 8"
[ 8 -gt 18 ] || echo "8 is not greater than 16"
[ -f /etc/passwd ] && echo "/etc/passwd exists"
[ -d /tmp ] && echo "/tmp is a directory"

echo "testing chaining"
false || echo "previous failed"
true && echo "this runs after success"

export COUNT=15
[ -n "$COUNT" ] && echo "COUNT is set"
[ "$COUNT" -gt 10 ] && echo "COUNT is greater than 10"
[ "$COUNT" = "15" ] && echo "COUNT equals 15"
[ -z "$EMPTYVAR" ] && echo "EMPTYVAR is empty or not set"


echo "testing redirection"
echo hello > test.txt
cat test.txt
echo "end of test"
