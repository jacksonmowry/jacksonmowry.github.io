#!/bin/sh

dotnet restore
dotnet tool install --global dotnet-ef
dotnet ef database update
