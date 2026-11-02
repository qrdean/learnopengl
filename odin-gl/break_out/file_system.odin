package main

import "core:fmt"
import "base:runtime"
import "core:io"
import "core:os"

read_entire_file :: proc(path: string, allocator: runtime.Allocator) -> ([]byte, bool) {
	content, err := os.read_entire_file(path, allocator)

	if err != nil {
		return {}, false
	}

	return content, true
}

File :: os.File
File_Error :: os.Error

file_open :: proc(filename: string) -> (^File, File_Error) {
	return os.open(filename)
}

file_read :: proc(f: ^File, p: []byte) -> (n: int, err: File_Error) {
	return os.read(f, p)
}

file_seek :: proc(f: ^File, offset: i64, whence: io.Seek_From) -> (ret: i64, err: File_Error) {
	return os.seek(f, offset, whence)
}

file_close :: proc(f: ^File) -> File_Error {
	return os.close(f)
}

require :: proc(ok: bool, what: string, loc := #caller_location) {
	if !ok {
		fmt.eprintf("%s(%d:%d): %s\n", loc.file_path, loc.line, loc.column, what)
		os.exit(1)
	}
}
