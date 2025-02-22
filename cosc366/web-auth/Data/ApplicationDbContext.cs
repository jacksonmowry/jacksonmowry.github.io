using App.Areas.Identity.Data;
using Microsoft.AspNetCore.Identity.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore;

namespace App.Data;

public class ApplicationDbContext : IdentityDbContext<ApplicationUser> {
    public ApplicationDbContext(DbContextOptions<ApplicationDbContext> options)
        : base(options) {
    }
}
