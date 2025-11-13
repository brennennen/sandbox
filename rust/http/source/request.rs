

#[derive(Debug)]
pub struct Request
{
    pub protocol: crate::Protocol,
    pub method: crate::Method,
    pub raw_path: String,
    pub headers: std::collections::HashMap<String, String>
}

impl Request {
    pub fn new(raw_request: &[i8] ) -> Request {
        let mut request = Request {
            protocol: crate::Protocol::new(),
            method: crate::Method::Unknown,
            raw_path: "".to_string(),
            headers: std::collections::HashMap::new()
        };
        request.read(raw_request);
        return request;
    }

    pub fn read(&mut self, raw_request: &[i8]) {
        let header_first_line_end = self.read_header_first_line(raw_request);
        let headers_end = self.read_headers(raw_request, header_first_line_end + 2);
        self.read_body(raw_request, headers_end);
        // TODO: parse body
    }

    ///
    /// Reads the first line of an http request.
    /// Per section 5.1 of the HTTP1.0 specification, the first line of
    /// a request shall contain the following:
    /// Method SP Request-URI SP HTTP-Version CRLF
    /// Where "SP" is the ascii space character (0x20).
    ///
    pub fn read_header_first_line(&mut self, raw_request: &[i8]) -> usize {
        // The first line is always an ascii string.
        let mut field_start: usize = 0;
        let mut field_end: usize = 0;
        let mut current_field = 0;
        for i in 0..raw_request.len() {
            // Check for CRLF and parse the http version.
            if raw_request[i] == 0x0D
                && i + 1 <= raw_request.len()
                && raw_request[i + 1] == 0x0A {
                field_end = i;
                let http_version = std::str::from_utf8(
                    unsafe {
                        &*(&raw_request[field_start..field_end] as *const _ as *const [u8])
                    }
                ).unwrap();
                self.protocol = http_version.parse().unwrap();
                break;
            }
            // Check for space and process the field preceding it.
            if raw_request[i] == 0x20 {
                // If this is the first space, the field is the method.
                if current_field == 0 {
                    field_end = i;
                    let method_string = std::str::from_utf8(
                        unsafe {
                            &*(&raw_request[field_start..field_end] as *const _ as *const [u8])
                        }
                    ).unwrap();
                    self.method = method_string.parse().unwrap();
                    field_start = field_end + 1;
                    current_field += 1;
                }
                // If this is the second space, the field is the path.
                else if current_field == 1 {
                    field_end = i;
                    self.raw_path = std::str::from_utf8(
                        unsafe {
                            &*(&raw_request[field_start..field_end] as *const _ as *const [u8])
                        }
                    ).unwrap().to_owned();
                    field_start = field_end + 1;
                    current_field += 1;
                }
            }
        }
        return field_end;
    }

    ///
    /// Read until there is an empty line.
    ///
    pub fn read_headers(&mut self, raw_request: &[i8], start_index: usize) -> usize {
        let mut current_index = start_index;
        let mut parsing_value = false;
        let mut key_start = start_index;
        let mut key_end = start_index;
        let mut current_key = "";

        // Parse each header line.
        loop {
            // Reached end of current header
            if raw_request.len() >= current_index + 1
                && raw_request[current_index] as u8 as char == '\r'
                && raw_request[current_index + 1] as u8 as char == '\n' {

                // Reached end of headers.
                if  raw_request.len() >= current_index + 3
                    && raw_request[current_index + 2] as u8 as char == '\r'
                    && raw_request[current_index + 3] as u8 as char == '\n' {
                    break;
                }

                // Reached end of current header
                else {
                    let value = std::str::from_utf8(
                        unsafe {
                            &*(&raw_request[key_end + 1..current_index] as *const _ as *const [u8])
                        }
                    ).unwrap();
                    self.headers.insert(current_key.to_string(), value.to_string());
                    key_start = current_index + 2;
                    parsing_value = false;
                }
            }

            // Reached end of key.
            if raw_request[current_index] as u8 as char == ':'
                && parsing_value == false{
                key_end = current_index;
                current_key = std::str::from_utf8(
                    unsafe {
                        &*(&raw_request[key_start..key_end] as *const _ as *const [u8])
                    }
                ).unwrap();
                parsing_value = true;
            }
            current_index += 1;
        }
        return current_index;
    }

    pub fn read_body(&mut self, _raw_request: &[i8], _start_index: usize) {
        // some http requests don't have bodies, check if start index is at the end of raw request
    }
}
