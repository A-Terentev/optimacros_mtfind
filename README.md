# optimacros_mtfind
Test task for position Cpp SDE - Search for a substring in a text file using a mask
Task description:

Write a program called mtfind to find a substring in a text file.
By mask using multithreading.
The mask is a string where "?" stands for any character.

The program accepts as command line parameters:
1) The name of the text file in which the search should go (file size - up to 1GB).
2) Search mask, in quotes. The maximum mask length is 100 characters.

The output of the program should be in the following format:
- On the first line - the number of found occurrences.
- Further information about each occurrence, each on a separate line, separated by a space:
line number, position in the line, the occurrence itself.

Additions:
- In a text file, encoding is only 7-bit ASCII
- Case sensitive search
- Each entry can only be on one line. The mask cannot contain a newline character
- Found occurrences must not intersect.
- Spaces and separators are included in the search along with other characters.
- You can use STL, Boost, C++1x features.
- Multithreading is a must. Single threaded solutions will not count.
- A serious plus will be the division of work between threads evenly, regardless of the number of lines in the input file.

EXAMPLE
input.txt file:

1 I've paid my dues
2 Time after time.
3 I've done my sentence
4 But committed no crime.
5 And bad mistakes?
6 I've made a few.
7 I've had my share of sand kicked in my face
8 But I've come through.

Run the program: mtfind input.txt "?ad"
Expected Result:

3
55 bad
66 mad
76 had

Present the solution in the form of an archive with source code and
CMake project
or Visual Studio (or as a link to an online Git repository).
The code must compile to GCC or MSVC.

Solution evaluation criteria:
1) The correctness of the output results
2) Quality and readability of the code, ease of further development and support
3) Speed and memory consumption
