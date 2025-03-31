use std::ffi::CString;
use std::os::raw::c_char;

#[no_mangle]
pub extern "C" fn free_rust_string(s: *mut c_char) {
    if !s.is_null() {
        unsafe {
            let _ = CString::from_raw(s);
        }
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
