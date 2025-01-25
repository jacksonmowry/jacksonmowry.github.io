using System.Security.Cryptography;
using System.Text.Json;
using Microsoft.Extensions.Logging;

internal sealed class EncryptedEchoServer : EchoServerBase {

    /// <summary>
    /// Logger to use in this class.
    /// </summary>
    private ILogger<EncryptedEchoServer> Logger { get; init; } =
        Settings.LoggerFactory.CreateLogger<EncryptedEchoServer>()!;

    /// <inheritdoc />
    internal EncryptedEchoServer(ushort port) : base(port) {
        rsa_instance = RSA.Create();
    }

    // todo: Step 1: Generate a RSA key (2048 bits) for the server.
           
    /// <inheritdoc />
    public override string GetServerHello() {
        // todo: Step 1: Send the public key to the client in PKCS#1 format.
        // Encode using Base64: Convert.ToBase64String
        return System.Convert.ToBase64String(rsa_instance.ExportRSAPublicKey());
    }

    /// <inheritdoc />
    public override string TransformIncomingMessage(string input) {
        // todo: Step 1: Deserialize the message.
        var message = JsonSerializer.Deserialize<EncryptedMessage>(input);
        
        // todo: Step 2: Decrypt the message using hybrid encryption.
        byte[] aes_key = rsa_instance.Decrypt(message.AesKeyWrap, System.Security.Cryptography.RSAEncryptionPadding.OaepSHA256);
        byte[] aes_iv = rsa_instance.Decrypt(message.AESIV, System.Security.Cryptography.RSAEncryptionPadding.OaepSHA256);
        byte[] hmac_key = rsa_instance.Decrypt(message.HMACKeyWrap, System.Security.Cryptography.RSAEncryptionPadding.OaepSHA256);

        Aes aes = Aes.Create();
        aes.Key = aes_key;
        aes.IV = aes_iv;

        byte[] decrypted = aes.DecryptCbc(message.Message, aes.IV);

        // todo: Step 3: Verify the HMAC.
        // Throw an InvalidSignatureException if the received hmac is bad.
        HMACSHA256 hmac = new HMACSHA256(hmac_key);
        byte[] hash = hmac.ComputeHash(decrypted);

        if (!hash.SequenceEqual(message.HMAC)) {
            Console.WriteLine(System.Text.Encoding.UTF8.GetString(hash));
            Console.WriteLine(System.Text.Encoding.UTF8.GetString(message.HMAC));
            throw new InvalidSignatureException();
        }

        // todo: Step 3: Return the decrypted and verified message from the server.
        return Settings.Encoding.GetString(decrypted);
    }

    /// <inheritdoc />
    public override string TransformOutgoingMessage(string input) {
        byte[] data = Settings.Encoding.GetBytes(input);

        // todo: Step 1: Sign the message.
        // Use PSS padding with SHA256.
        byte[] signed = rsa_instance.SignData(data, System.Security.Cryptography.HashAlgorithmName.SHA256, System.Security.Cryptography.RSASignaturePadding.Pss);

        // todo: Step 2: Put the data in an SignedMessage object and serialize to JSON.
        // Return that JSON.
        var message = new SignedMessage(data, signed);
        return JsonSerializer.Serialize(message);
    }

    private RSA rsa_instance;
}
