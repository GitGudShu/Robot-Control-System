FILES=("maker_robots.c" "painter_robots.c" "transporter_robots.c" "server.c")
EXECUTABLES=("makers" "painters" "transporters" "server")

echo "Compiling source files into separate executables..."
for i in "${!FILES[@]}"; do
    gcc "${FILES[i]}" -o "${EXECUTABLES[i]}"
    if [ $? -eq 0 ]; then
        echo "Compiled ${FILES[i]} -> ${EXECUTABLES[i]}"
    else
        echo "Compilation failed for ${FILES[i]}!"
        exit 1
    fi
done
