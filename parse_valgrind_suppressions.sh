#!/bin/bash

# Script to parse Valgrind output and create suppressions file
# Usage: ./parse_valgrind_suppressions.sh < valgrind_output.txt

echo "Creating Valgrind suppressions file..."

# Create suppressions file
cat > valgrind_suppressions.txt << 'EOF'
# Suppress SFML internal leaks (these are from the SFML library, not our code)
{
   SFML_Internal_Leaks
   Memcheck:Leak
   ...
   fun:*SFML*
}

# Suppress font loading leaks
{
   Font_Loading_Leaks
   Memcheck:Leak
   ...
   fun:*font*
}

# Suppress OpenGL context leaks
{
   OpenGL_Context_Leaks
   Memcheck:Leak
   ...
   fun:*gl*
}

# Suppress X11 leaks (common on Linux)
{
   X11_Leaks
   Memcheck:Leak
   ...
   fun:*X11*
}

# Suppress pthread leaks
{
   Pthread_Leaks
   Memcheck:Leak
   ...
   fun:*pthread*
}
EOF

echo "Valgrind suppressions file created: valgrind_suppressions.txt"
echo "To use with Valgrind:"
echo "valgrind --suppressions=valgrind_suppressions.txt ./build/bin/Project1"
