using App.Areas.Identity.Data;
using Microsoft.AspNetCore.Identity;

namespace App.Areas.Identity;

public class PasswordHasher : IPasswordHasher<ApplicationUser> {

    /// <summary>
    /// Hash a password using the appropriate method.
    /// </summary>
    /// <param name="password"></param>
    /// <returns></returns>
    public string HashPassword(ApplicationUser user, string password) =>
        user.PasswordHasherAlgorithm switch {
            PasswordHasherAlgorithm.Iterative => new IterativeHasher().HashPassword(user, password),
            PasswordHasherAlgorithm.PBKDF2 => new PBKDF2Hasher().HashPassword(user, password),
            PasswordHasherAlgorithm.BCrypt => new BCryptHasher().HashPassword(user, password),
            PasswordHasherAlgorithm.Argon2id => new Argon2idHasher().HashPassword(user, password),
            _ => throw new ArgumentException("User has an invalid password hashing algorithm set.", nameof(user))
        };

    /// <summary>
    /// Verify that a password matches the hashedPassword using the appropriate method.
    /// </summary>
    /// <param name="password"></param>
    /// <returns></returns>
    public PasswordVerificationResult VerifyHashedPassword(
        ApplicationUser user, string hashedPassword, string providedPassword) =>
        user.PasswordHasherAlgorithm switch {
            PasswordHasherAlgorithm.Iterative => new IterativeHasher().VerifyHashedPassword(
                user, hashedPassword, providedPassword),
            PasswordHasherAlgorithm.PBKDF2 => new PBKDF2Hasher().VerifyHashedPassword(
                user, hashedPassword, providedPassword),
            PasswordHasherAlgorithm.BCrypt => new BCryptHasher().VerifyHashedPassword(
                user, hashedPassword, providedPassword),
            PasswordHasherAlgorithm.Argon2id => new Argon2idHasher().VerifyHashedPassword(
                user, hashedPassword, providedPassword),
            _ => throw new ArgumentException("User has an invalid password hashing algorithm set.", nameof(user))
        };

}