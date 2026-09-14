# Mysh C++ Version

As a learning experience, I wanted to try reimplementing mysh in modern C++. Perhaps I'll also add Windows support. Currently in progress.

## Extensions over the original C shell
Now supports quotes and semicolons the same way bash does.

## Modern C++ Features
While the original C shell would compile with a C++ compiler with few changes, I wanted to do a full rewrite in modern idiomatic C++. Here are a few things I used

## Ranges Library
The tokenizer now uses views from the C++20/23 ranges library instead of doing it by hand

## Filesystem Library
Instead of directly making POSIX system calls like opendir and chdir, I now use the C++17 filesystem library.

## Optional
Whenever a function can fail, I prefer using the std::optional wrapper and returning std::nullopt as opposed to raw NULL or nullptr.
