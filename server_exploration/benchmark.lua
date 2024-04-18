-- Function to handle errors
function error_handler(err)
    io.stderr:write("Read error: " .. err .. "\n")
    os.exit(1)
end

-- Main function to perform HTTP requests
function request()
    local response = wrk.format("GET", "/http_uring_rough.c")  -- Change "/your-endpoint-here" to the endpoint you want to test
    local ok, err = pcall(wrk.send, response)  -- Try sending the request
    if not ok then
        error_handler(err)  -- If there's an error, handle it
    end
end
