#!/usr/bin/env bash

for cmd in tokei jq gum; do
  if ! command -v "$cmd" &> /dev/null; then
    echo "Error: $cmd is not installed. Please install it first."
    exit 1
  fi
done

tokei -e "replxx" -o json | jq -r '
del(.Total) | 
to_entries | 
[
  ["Language", "Files", "Lines", "Code", "Comments", "Blanks"],
  (.[] | select(.key != "Total") | [
    .key,
    (.value.reports | length | tostring),
    ((.value.code + .value.comments + .value.blanks) | tostring),
    (.value.code | tostring),
    (.value.comments | tostring),
    (.value.blanks | tostring)
  ]),
  ["---", "---", "---", "---", "---", "---"],
  ["TOTAL",
    (map(.value.reports | length) | add | tostring),
    (map(.value.code + .value.comments + .value.blanks) | add | tostring),
    (map(.value.code) | add | tostring),
    (map(.value.comments) | add | tostring),
    (map(.value.blanks) | add | tostring)
  ]
] | .[] | @tsv
' | gum table \
  --separator="	" \
  --widths=20,8,8,8,10,8 \
  --print
