OCR Server
==========
OCR Server is a simple stand-alone RESTful API for converting images of typed, handwritten or printed text into plain text, powered by [Tesseract](https://github.com/tesseract-ocr/tesseract) open source OCR engine.

Preperation of Tesseract
------------------------
Get the language files of Tesseract from https://github.com/tesseract-ocr/tessdata.
Make sure the `TESSDATA_PREFIX` environment variable is set to your `tessdata` directory.

Configuration File
------------------
The server settings can be specified using an optional JSON file named `ocrServer.config.json`.
All fields are optional.

```javascript
{
  // The IP address or host name of the server to listen to
  "hostname": "localhost",
  
  // The port number of the server to listen to
  "port": 8080,
  
  // Path to the file containing the SSL certificate (PEM format)
  "ssl_certificate": "cert.pem",
  
  // Path to the file containing the SSL private key (PEM format)
  "ssl_private_key": "key.pem",
  
  // An array of authorized API keys
  // Do not specify any API key to allow anonymous access
  "api_keys": [
    "Key 1",								        
    "Key 2"
  ]
}
```

Command Line Options
--------------------
The command line options will overwrite the settings specified by the configuration file.
The following command line options are available:

| Option         | Argument    | Default | Description                                                  |
|----------------|-------------|---------|--------------------------------------------------------------|
|     --help	   |             |         | Displays the list of command line options                    |
| -h, --hostname | Name or IP  | 0.0.0.0 | The IP address or host name of the server to listen to       |
| -p, --port     | Port Number | 8080    | The port number of the server to listen to                   |
| -c, --sslcert  | File Path   |         | Path to the file containing the SSL certificate (PEM format) |
| -p, --sslkey   | File Path   |         | Path to the file containing the SSL private key (PEM format) |
| -a, --apikey   | API Key     |         | The authorized API key, multiple keys are allowed            |


API Endpoints
-------------

* GET `/ocr/languages`
  Returns the sorted list of available Tesseract languages.

* POST `/ocr/{LANGUAGE}`
  Extracts and returns text from a given image in the specified language.
  
  - `LANGUAGE`: one of the available Tesseract languages (case sensitive)
  - Content-Type Header: an image mime type (image/png, image/gif, image/bmp, image/jpeg, image/tiff, or image/webp)
  - Request Body: the image as binary


If the server setting contains any API key, the API calls must be authorized. To do so, use the `Authorization` header 
and `APIKEY` authentication scheme to pass an API key to the server. For example, if `1234` is defined as an API key, 
the following HTTP header should be present in any request:
```
Authorization: APIKEY 1234
```
