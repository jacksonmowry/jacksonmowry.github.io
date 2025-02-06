using System.Security.Cryptography;
using System.Text.Json;
using Microsoft.Extensions.Logging;

/// <summary>
/// Provides a base class for implementing an Echo client.
/// </summary>
internal sealed class EncryptedEchoClient : EchoClientBase {

    /// <summary>
    /// Logger to use in this class.
    /// </summary>
    private ILogger<EncryptedEchoClient> Logger { get; init; } =
        Settings.LoggerFactory.CreateLogger<EncryptedEchoClient>()!;

    /// <inheritdoc />
    public EncryptedEchoClient(ushort port, string address) : base(port, address) {
        rsa_instance = RSA.Create();
    }

    /// <inheritdoc />
    public override void ProcessServerHello(string message) {
        // todo: Step 1: Get the server's public key. Decode using Base64.
        // Throw a CryptographicException if the received key is invalid.
        byte[] bytes = new byte[Convert.ToInt32(message.Length * 1.5)];
        int bytes_written = 0;
        bool result = System.Convert.TryFromBase64String(message, bytes, out bytes_written);
        if (!result) {
            throw new CryptographicException();
        }

        server_rsa_pub_key = bytes;

        int bytes_read = 0;
        rsa_instance.ImportRSAPublicKey(bytes, out bytes_read);
    }

    /// <inheritdoc />
    public override string TransformOutgoingMessage(string input) {
        byte[] data = Settings.Encoding.GetBytes(input);

        // todo: Step 1: Encrypt the input using hybrid encryption.
        // Encrypt using AES with CBC mode and PKCS7 padding.
        // Use a different key each time.
        Aes aes = Aes.Create();

        byte[] encrypted = aes.EncryptCbc(data, aes.IV);

        // todo: Step 2: Generate an HMAC of the message.
        // Use the SHA256 variant of HMAC.
        // Use a different key each time.
        byte[] secretKey = new Byte[64];
        RandomNumberGenerator rng = RandomNumberGenerator.Create();
        rng.GetBytes(secretKey);
        HMACSHA256 hmac = new HMACSHA256(secretKey);
        byte[] hash = hmac.ComputeHash(data);

        // todo: Step 3: Encrypt the message encryption and HMAC keys using RSA.
        // Encrypt using the OAEP padding scheme with SHA256.
        byte[] aes_key = rsa_instance.Encrypt(aes.Key, System.Security.Cryptography.RSAEncryptionPadding.OaepSHA256);
        byte[] aes_iv = rsa_instance.Encrypt(aes.IV, System.Security.Cryptography.RSAEncryptionPadding.OaepSHA256);
        byte[] hmac_key = rsa_instance.Encrypt(secretKey, System.Security.Cryptography.RSAEncryptionPadding.OaepSHA256);

        // todo: Step 4: Put the data in an EncryptedMessage object and serialize to JSON.
        // Return that JSON.
        var message = new EncryptedMessage(aes_key, aes_iv, encrypted, hmac_key, hash);
        return JsonSerializer.Serialize(message);
    }

    /// <inheritdoc />
    public override string TransformIncomingMessage(string input) {
        // todo: Step 1: Deserialize the message.
        var signedMessage = JsonSerializer.Deserialize<SignedMessage>(input);

        // todo: Step 2: Check the messages signature.
        // Use PSS padding with SHA256.
        // Throw an InvalidSignatureException if the signature is bad.
        if (!rsa_instance.VerifyData(signedMessage.Message, signedMessage.Signature, System.Security.Cryptography.HashAlgorithmName.SHA256, System.Security.Cryptography.RSASignaturePadding.Pss)) {
            throw new InvalidSignatureException();
        }

        // todo: Step 3: Return the message from the server.
        return Settings.Encoding.GetString(signedMessage.Message);
    }

    private RSA rsa_instance;
    private byte[] server_rsa_pub_key;
}
