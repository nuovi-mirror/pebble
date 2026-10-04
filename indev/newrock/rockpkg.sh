#!/bin/sh

file=$1

if [ $# -ne 1 ]; then
	echo "usage: $0 package.rockpkg" >&2
	exit 1
fi

if [ ! -f "$file" ]; then
	echo "rockpkg: file not found: $file" >&2
	exit 1
fi

rockdir="$HOME/.rock/"
appsdir="$rockdir/apps/"

parsefile() {
	while IFS= read -r line || [ -n "$line" ]; do
		case "$line" in
			''|'#'*) continue ;;
		esac

		# src src/foo.newrock = [[https://foo.com/foo.newrock]]
		case "$line" in
			"src "*)
				rest=${line#src }
				path=${rest%% = *}
				url=${rest#* = }
				url=${url#'[['}
				url=${url%']]'}
				printf 'SRC: %s -> %s\n' "$path" "$url"

				src_paths="$src_paths
$path"
				src_urls="$src_urls
$url"
				;;

			*=*)
				key=${line%% = *}
				value=${line#* = }

				case "$key" in
					name) name=$value ;;
					descr) descr=$value ;;
					longdescr) longdescr=$value ;;
					version) version=$value ;;
					contact) contact=$value ;;
					contacttype) contacttype=$value ;;
					program) program=$value ;;
					includedir) includedir=$value ;;
					output) output=$value ;;
					*)
						echo "rockpkg: warning: unknown key: $key" >&2
						;;
				esac
				;;

			*)
				echo "rockpkg: malformed line: $line" >&2
				exit 1
				;;
		esac

	done < "$file"
}

parsefile

printf '\n'
printf 'name:        %s\n' "$name"
printf 'description: %s\n' "$descr"
printf 'version:     %s\n' "$version"
printf 'contact:     %s (%s)\n' "$contact" "$contacttype"
printf 'program:     %s\n' "$program"
printf 'include:     %s\n' "$includedir"
printf 'output:      %s\n' "$output"

echo
printf "Do you wish to install this? (Y/n): "
read input

if [ "$input" != "Y" ]; then
	echo "Abort!"
	exit 0
fi

mkdir -p "$appsdir"

pkgdir="$appsdir/$name/$version"

# skip check
#if [ -e "$pkgdir" ]; then
#	echo "rockpkg: package already exists: $pkgdir" >&2
#	exit 1
#fi

mkdir -p "$pkgdir"
cd "$pkgdir" || exit 1

# Fetch package sources.
old_ifs=$IFS
IFS='
'

set -- $src_paths
paths=$*

set -- $src_urls
urls=$*

IFS=$old_ifs

i=1

for path in $paths; do
	url=$(printf '%s\n' "$urls" | sed -n "${i}p")

	mkdir -p "$(dirname "$path")"

	echo "Fetching $path..."
	curl -fL -o "$path" "$url" || exit 1

	i=$((i + 1))
done

mkdir -p "$(dirname "$output")"

echo "Compiling $name $version..."

printf '#!/usr/bin/env pblvm\n' > "$output"

rockc "$program" "$includedir" >> "$output"

if [ $? -ne 0 ]; then
	echo "rockpkg: compilation failed" >&2
	rm -f "$output"
	exit 1
fi

chmod +x "$output"

echo "Built $output"

mkdir -p "$rockdir/bin"
ln -sf "$pkgdir/$output" "$rockdir/bin/$name"

echo "Installed $name Version $version"
