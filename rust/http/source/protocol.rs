///
/// HTTP specification associated with the request or response.
/// 
/// HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT
/// 
/// Their are 2 major HTTP specification branches at the moment.
///  * HTTP/1.1
///  * HTTP/2.0
/// 


#[derive(Debug)]
pub struct Protocol
{
    pub major: u32,
    pub minor: u32
}

impl std::str::FromStr for Protocol {
    type Err = String;

    fn from_str(protocol_string: &str) -> Result<Self, Self::Err> {
        let mut protocol: Protocol = Protocol::new();
        let slash_index = protocol_string.find('/');
        let dot_index = protocol_string.find('.');

        match protocol_string[slash_index.unwrap()+1..dot_index.unwrap()].parse::<u32>() {
            Ok(number) => protocol.major = number,
            Err(e) => return Err(format!("'{}' is not a valid http protocol string. Inner Error: {}", protocol_string, e.to_string()))
        }

        match protocol_string[dot_index.unwrap()+1..protocol_string.len()].parse::<u32>() {
            Ok(number) => protocol.minor = number,
            Err(e) => return Err(format!("'{}' is not a valid http protocol string. Inner Error: {}", protocol_string, e.to_string()))
        }

        return Ok(protocol);
    }
}

impl std::fmt::Display for Protocol {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        return write!(f, "HTTP/{}.{}", self.major, self.minor);
    }
}

impl Protocol {
    pub fn new() -> Protocol {
        let protocol = Protocol {
            major: 0,
            minor: 0
        };
        return protocol;
    }
}
