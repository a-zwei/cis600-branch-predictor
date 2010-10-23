#!/bin/sh

branches() {
  wc -l <$1
}

hits() {
  grep hit $1 | wc -l
}

correct() {
  egrep "(1\s*taken: 1)|(0\s*taken: 0)" $1
}

branch_addresses() {
  awk '{ print $1 }' history.txt
}

most_common_branch() {
  for branch in `branch_addresses | sort -u`
  do
    echo `branch_addresses | grep "$branch" | wc -l` $branch
  done | sort -n -r | head -1 | awk '{ print $2 }'
}

branch_count() {
  branch_addresses | grep $1 | wc -l
}

unique_branches() {
  branch_addresses | sort -u | wc -l
}

echo "`branches $1` branches"
echo "`hits $1` hits"
echo "`correct $1 | wc -l` correct"
msb=`most_common_branch`
echo "most common branch: $msb, `branch_count $msb` times"
echo "    correct" `correct $1 | grep "addr: $msb" | wc -l`
echo "unique branches" `unique_branches`
