using System.Security.Cryptography;
using Microsoft.AspNetCore.Identity;

namespace App.Areas.Identity;

/// <summary>
/// Password hasher backed by PBKDF2.
/// </summary>
/// <remarks>
/// For reference, consider the <see href="https://github.com/aspnet/AspNetIdentity/blob/main/src/Microsoft.AspNet.Identity.Core/PasswordHasher.cs">default implementation</see>
/// </remarks>
internal class PBKDF2Hasher : IPasswordHasher<IdentityUser> {

    /// <summary>
    /// Hash a password using PBKDF2.
    /// </summary>
    /// <param name="password">Password to hash.</param>
    /// <returns>String containing all the information needed to verify the password in the future.</returns>
    public string HashPassword(IdentityUser user, string password) {
        // todo: Use a random 32-byte salt. Use a 32-byte digest.
        // todo: Use 100,000 iterations and the SHA256 algorithm.
        // todo: Encode as "Base64(salt):Base64(digest)"
        byte[] salt = RandomNumberGenerator.GetBytes(32);
        byte[] digest = Rfc2898DeriveBytes.Pbkdf2(System.Text.Encoding.ASCII.GetBytes(password), salt, 100000, System.Security.Cryptography.HashAlgorithmName.SHA256, 32);
        return Utils.EncodeSaltAndDigest(salt, digest);
    }

    /// <summary>
    /// Verify that a password matches the hashed password.
    /// </summary>
    /// <param name="hashedPassword">Hashed password value stored when registering.</param>
    /// <param name="providedPassword">Password provided by user in login attempt.</param>
    /// <returns></returns>
    public PasswordVerificationResult VerifyHashedPassword(IdentityUser user, string hashedPassword, string providedPassword) {
        // todo: Verify that the given password matches the hashedPassword (as originally encoded by HashPassword)
        (byte[] salt, byte[] digest) = Utils.DecodeSaltAndDigest(hashedPassword);

        byte[] new_digest = Rfc2898DeriveBytes.Pbkdf2(System.Text.Encoding.ASCII.GetBytes(providedPassword), salt, 100000, System.Security.Cryptography.HashAlgorithmName.SHA256, 32);

        if (!digest.SequenceEqual(new_digest)) {
            return PasswordVerificationResult.Failed;
        }

        return PasswordVerificationResult.Success;
    }

}
