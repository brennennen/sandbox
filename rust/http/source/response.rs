
pub struct Response
{
    pub status_code: crate::StatusCode,
    pub content: Vec<u8>,
    pub data: Vec<u8>
}

impl Response {
    pub fn new() -> Response {
        Response {
            status_code: crate::StatusCode::Unknown,
            content: Vec::new(),
            data: Vec::new()
        }
    }

    pub fn set_content(&mut self, _data: Vec<u8>) {
        // TODOA
    }

    pub fn set_content_text(&mut self, text: String) {
        self.content = text.as_bytes().to_vec();
        // self.content.extend_from_slice(text.as_bytes());
    }

    /// 
    /// Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
    /// 
    pub fn write_status_line(&mut self) {
        let example_status_line = "HTTP/1.1 200 OK \r\n";
        self.data.extend_from_slice(example_status_line.as_bytes());
    }

    pub fn write_headers(&mut self) {
        let content_type = "text/html";
        let content_length = self.content.len();
        let headers = format!("\
        Content-Type: {}\n\
        Content-Length: {}\n\
        \n\
        ", content_type, content_length);
        //let headers = format!("Content-Type: text/html \r\nContent-Length: 98\r\n\r\n");
        self.data.extend_from_slice(headers.as_bytes());
    }

    pub fn write_content(&mut self) {
        //self.data.extend_from_slice(self.content.as_bytes());
        //self.data = text.as_bytes().to_vec();
        self.data.extend(self.content.iter().cloned());
    }

    pub fn write_response(&mut self) {
        self.write_status_line();
        self.write_headers();
        self.write_content();
    }
}

pub fn read()
{
    
}

// mod Verb;

// struct Request
// {
//     verb1: verb
// }