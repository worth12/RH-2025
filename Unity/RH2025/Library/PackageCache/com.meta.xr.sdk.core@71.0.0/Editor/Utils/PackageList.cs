/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * Licensed under the Oculus SDK License Agreement (the "License");
 * you may not use the Oculus SDK except in compliance with the License,
 * which is provided at the time of installation or download, or which
 * otherwise accompanies this software in either electronic or hard copy form.
 *
 * You may obtain a copy of the License at
 *
 * https://developer.oculus.com/licenses/oculussdk/
 *
 * Unless required by applicable law or agreed to in writing, the Oculus SDK
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

using System;
using System.Linq;
using System.Text.RegularExpressions;
using System.Threading;
using UnityEditor;
using UnityEditor.PackageManager;
using UnityEditor.PackageManager.Requests;
using PackageInfo = UnityEditor.PackageManager.PackageInfo;

namespace Meta.XR.Editor.Utils
{
    [InitializeOnLoad]
    internal static class PackageList
    {
        private static ListRequest _packageManagerListRequest;

        static PackageList()
        {
            RefreshPackageList(false);
        }

        public static bool PackageManagerListAvailable => _packageManagerListRequest.Status == StatusCode.Success;

        public static PackageInfo GetPackage(string packageId)
        {
            if (!PackageManagerListAvailable || _packageManagerListRequest.Result == null)
            {
                return null;
            }

            var (name, _) = ParsePackageId(packageId);
            return _packageManagerListRequest.Result.FirstOrDefault(p => p.name == name);
        }

        private static (string name, string version) ParsePackageId(string packageId)
        {
            var packageNameParts = packageId.Split("@");
            var packageName = packageNameParts[0];
            var packageVersion = packageNameParts.Length > 1 ? packageNameParts[1] : null;
            return (packageName, packageVersion);
        }

        public static bool IsPackageInstalled(string packageId) => GetPackage(packageId) != null;

        public static bool IsPackageInstalledWithValidVersion(string packageId)
        {
            var (_, expectedPackageVersion) = ParsePackageId(packageId);
            var installedPacked = GetPackage(packageId);

            if (installedPacked == null)
            {
                return false;
            }

            if (string.IsNullOrEmpty(expectedPackageVersion))
            {
                return true;
            }

            return IsVersionValid(expectedPackageVersion, installedPacked.version);
        }

        internal static bool IsValidPackageName(string packageName)
        {
            const string pattern = @"^([a-z0-9]+(-[a-z0-9]+)*\.)+[a-z]{2,}(@([0-9]+\.){2}[0-9]+(-[0-9A-Za-z-]+(\.[0-9A-Za-z-]+)*)?(\+[0-9A-Za-z-]+(\.[0-9A-Za-z-]+)*)?)?$";
            var regex = new Regex(pattern, RegexOptions.IgnoreCase);
            return regex.IsMatch(packageName);
        }

        internal static bool IsValidPackageId(string packageId)
        {
            var (packageName, packageVersion) = ParsePackageId(packageId);
            if (!IsValidPackageName(packageName))
            {
                return false;
            }

            return !packageId.Contains("@") || IsValidSemanticVersion(packageVersion);
        }

        internal static bool IsValidSemanticVersion(string version)
        {
            if (string.IsNullOrEmpty(version))
            {
                return false;
            }

            // Regular expression for semantic versioning with support for ~ and ^ notations
            const string regex = @"^[\^~]?(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)" +
                              @"(?:-([0-9A-Za-z-]+(?:\.[0-9A-Za-z-]+)*))?" +
                              @"(?:\+([0-9A-Za-z-]+(?:\.[0-9A-Za-z-]+)*))?$";

            return Regex.IsMatch(version, regex);
        }

        internal static bool IsVersionValid(string expectedVersion, string actualVersion)
        {
            if (!IsValidSemanticVersion(expectedVersion) || !IsValidSemanticVersion(actualVersion))
                return false;

            var normalizedExpectedVersion = NormalizeVersion(expectedVersion);
            var normalizedActualVersion = NormalizeVersion(actualVersion);

            if (!Version.TryParse(normalizedExpectedVersion, out var expected) ||
                !Version.TryParse(normalizedActualVersion, out var actual))
            {
                return false;
            }

            if (expectedVersion.StartsWith("~"))
            {
                return expected.Major == actual.Major && expected.Minor == actual.Minor &&
                       actual.Build >= expected.Build;
            }

            if (expectedVersion.StartsWith("^"))
            {
                return expected.Major == actual.Major && actual.Minor >= expected.Minor &&
                       (expected.Minor != actual.Minor || actual.Build >= expected.Build);
            }

            return expected.Equals(actual);

            string NormalizeVersion(string version)
            {
                return version.Replace("^", "").Replace("~", "");
            }
        }

        private static void RefreshPackageList(bool blocking)
        {
            _packageManagerListRequest = Client.List(offlineMode: false, includeIndirectDependencies: true);

            if (!blocking)
            {
                return;
            }

            while (!PackageManagerListAvailable)
            {
                Thread.Sleep(100);
            }
        }

        public static void UninstallPackage(string packageName)
        {
            var request = Client.Remove(packageName);

            while (!request.IsCompleted)
            {
                Thread.Sleep(1);
            }

            RefreshPackageList(false);
        }
    }
}
