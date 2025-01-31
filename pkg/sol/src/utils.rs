use std::ffi::CString;
use std::os::raw::c_char;

/// Frees a Rust-allocated CString that was passed to C.
///
/// # Safety
/// - The `s` parameter must be a valid pointer returned by `CString::into_raw`.
/// - Passing a null pointer is a no-op.
/// - Undefined behavior may occur if `s` was not allocated by Rust's `CString::into_raw`.
#[no_mangle]
pub extern "C" fn free_rust_string(s: *mut c_char) {
    if s.is_null() {
        return;
    }
    unsafe {
        let c_string = CString::from_raw(s);
        std::mem::drop(c_string);
    }
}

#[cfg(test)]
#[allow(unused_unsafe)]
mod tests {
    use super::*;
    use std::ffi::CString;
    use std::os::raw::c_char;

    #[test]
    fn test_free_rust_string() {
        let original = CString::new("Hello, Rust!").expect("CString::new failed");

        let ptr: *mut c_char = original.into_raw();

        assert!(!ptr.is_null());

        unsafe {
            free_rust_string(ptr);
        }
    }
}
