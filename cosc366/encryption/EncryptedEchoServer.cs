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
        rsa_instance = RSA.Create(2048);
    }

    /// <inheritdoc />
    public override string GetServerHello() {
        // Encode using Base64: Convert.ToBase64String
        return System.Convert.ToBase64String(rsa_instance.ExportRSAPublicKey());
    }

    /// <inheritdoc />
    public override string TransformIncomingMessage(string input) {
        var message = JsonSerializer.Deserialize<EncryptedMessage>(input);
        
        byte[] aes_key = rsa_instance.Decrypt(message.AesKeyWrap, System.Security.Cryptography.RSAEncryptionPadding.OaepSHA256);
        byte[] hmac_key = rsa_instance.Decrypt(message.HMACKeyWrap, System.Security.Cryptography.RSAEncryptionPadding.OaepSHA256);

        Aes aes = Aes.Create();
        aes.Key = aes_key;

        byte[] decrypted = aes.DecryptCbc(message.Message, message.AESIV);

        // Throw an InvalidSignatureException if the received hmac is bad.
        HMACSHA256 hmac = new HMACSHA256(hmac_key);
        byte[] hash = hmac.ComputeHash(decrypted);

        if (!hash.SequenceEqual(message.HMAC)) {
            throw new InvalidSignatureException();
        }

        return Settings.Encoding.GetString(decrypted);
    }

    /// <inheritdoc />
    public override string TransformOutgoingMessage(string input) {
        byte[] data = Settings.Encoding.GetBytes(input);

        // Use PSS padding with SHA256.
        byte[] signed = rsa_instance.SignData(data, System.Security.Cryptography.HashAlgorithmName.SHA256, System.Security.Cryptography.RSASignaturePadding.Pss);

        // Return that JSON.
        var message = new SignedMessage(data, signed);
        return JsonSerializer.Serialize(message);
    }

    private RSA rsa_instance;
}
