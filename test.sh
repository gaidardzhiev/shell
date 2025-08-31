printf "\nStarting tests for conditional expressions using [ ... ]:\n"

[ 16 -gt 8 ] && printf "\tPASS: 16 is greater than 8\n" || printf "\tFAIL: 16 is NOT greater than 8\n"
[ 8 -gt 18 ] && printf "\tFAIL: 8 is greater than 18 (unexpected)\n" || printf "\tPASS: 8 is NOT greater than 18\n"
[ -f /etc/passwd ] && printf "\tPASS: /etc/passwd file exists\n" || printf "\tFAIL: /etc/passwd file does NOT exist\n"
[ -d /tmp ] && printf "\tPASS: /tmp is a directory\n" || printf "\tFAIL: /tmp is NOT a directory\n"

printf "\nTesting command chaining with AND (&&) and OR (||):\n"
false || printf "\tPASS: previous command failed, OR chain executed\n"
true && printf "\tPASS: previous command succeeded, AND chain executed\n"

printf "\nTesting variables and comparisons:\n"
export COUNT=15
[ -n "$COUNT" ] && printf "\tPASS: COUNT variable is set\n" || printf "\tFAIL: COUNT variable is NOT set\n"
[ "$COUNT" -gt 10 ] && printf "\tPASS: COUNT is greater than 10\n" || printf "\tFAIL: COUNT is NOT greater than 10\n"
[ "$COUNT" = "15" ] && printf "\tPASS: COUNT equals 15\n" || printf "\tFAIL: COUNT does NOT equal 15\n"
[ -z "$EMPTYVAR" ] && printf "\tPASS: EMPTYVAR is empty or unset\n" || printf "\tFAIL: EMPTYVAR is NOT empty\n"

printf "\nTesting output redirection:\n"
printf "\tWriting 'Hello, World!' to test.txt\n"
printf "\tHello, World!\n" > test.txt

[ -f test.txt ] && {
	printf "\tPASS: test.txt created\n"
	printf "\tContents of test.txt:\n"
	cat test.txt
	rm test.txt
} || printf "\tFAIL: test.txt was not created\n"

printf "\nEnd of test script.\n"
