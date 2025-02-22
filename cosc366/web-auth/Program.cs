using Microsoft.AspNetCore.Identity;
using Microsoft.EntityFrameworkCore;
using App.Data;
using App.Areas.Identity.Data;
using App.Areas.Identity;

var builder = WebApplication.CreateBuilder(args);

/******************************************************************************
 * Add services to the container.
 *****************************************************************************/
builder.Services.AddLogging(loggingBuilder => {
    var loggingSection = builder.Configuration.GetSection("Logging");
    loggingBuilder.AddFile(loggingSection, options => {
        options.FormatLogEntry = message => $"{message.LogLevel}: {message.Message}";
    });
});

var connectionString = builder.Configuration.GetConnectionString("DefaultConnection") ?? throw new InvalidOperationException("Connection string 'DefaultConnection' not found.");
builder.Services.AddDbContext<ApplicationDbContext>(options =>
    options.UseSqlite(connectionString));
builder.Services.AddDatabaseDeveloperPageExceptionFilter();

builder.Services.AddDefaultIdentity<ApplicationUser>(options => options.SignIn.RequireConfirmedAccount = false)
    .AddEntityFrameworkStores<ApplicationDbContext>();

builder.Services.AddTransient<IPasswordHasher<ApplicationUser>, PasswordHasher>();

builder.Services.AddRazorPages();

/******************************************************************************
 * Configure services in the container.
 *****************************************************************************/

builder.Services.Configure<IdentityOptions>(options => {
    // ! These are not appropriate options. I'm only using them here to make it easy to test the app.
    options.Password.RequireDigit = false;
    options.Password.RequiredLength = 1;
    options.Password.RequireLowercase = false;
    options.Password.RequireNonAlphanumeric = false;
    options.Password.RequireUppercase = false;

    options.User.RequireUniqueEmail = false;
});

builder.Services.ConfigureApplicationCookie(options => {
    options.Cookie.HttpOnly = true;
    options.ExpireTimeSpan = TimeSpan.FromMinutes(5);

    options.LoginPath = "/Identity/Account/Login";
    options.AccessDeniedPath = "/Identity/Account/AccessDenied";
    options.SlidingExpiration = true;
});

var app = builder.Build();

/******************************************************************************
 * Configure the HTTP request pipeline.
 *****************************************************************************/

if (app.Environment.IsDevelopment()) {
    app.UseMigrationsEndPoint();
} else {
    app.UseExceptionHandler("/Error");
    // The default HSTS value is 30 days. You may want to change this for production scenarios, see https://aka.ms/aspnetcore-hsts.
    app.UseHsts();
}

app.UseHttpsRedirection();
app.UseStaticFiles();

app.UseRouting();

app.UseAuthentication();
app.UseAuthorization();

app.MapRazorPages();

app.Run();
