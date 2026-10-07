# Yassuh fork

This is the yassuh fork of [Rallista/valhalla-mobile](https://github.com/Rallista/valhalla-mobile).
It stays as close to upstream as possible and adds one thing:
CI publishes the iOS and Android builds to this repository's GitHub releases,
using only the workflow's `GITHUB_TOKEN`.

## What differs from upstream

- `.github/workflows/release-yassuh.yml` builds, tests, and publishes releases.
- Upstream's `release.yml` and `docs.yml` skip outside Rallista/valhalla-mobile.
  The release needs upstream's Maven Central and signing secrets and a token that pushes to `main`,
  and the docs deploy to upstream's GitHub Pages site.
- `ios.yml` and `android.yml` can also be called by another workflow,
  so a release ships the exact binaries that those workflows built and tested.
- `scripts/write_xcframework_spm.sh` points `Package.swift` at the releases of the repository it runs in.

Everything else, including `version.txt` and the `Package.swift` on `main`, is upstream's.

## Release tags

A release is tagged `<version.txt>-yassuh.<revision>`.
For example, `0.6.4-yassuh.1` is the first fork release of upstream 0.6.4.
Raise the revision for each fork release of the same upstream version,
and start again at 1 after merging a new upstream version.
Published tags never move, so pin exact versions.
The `-yassuh` suffix is a semantic-versioning pre-release label,
so version-range tools sort it before the plain upstream version.

## Cutting a release

Run the **Yassuh release** workflow from `main` with the next revision:

```sh
gh workflow run release-yassuh.yml -R yassuh/valhalla-mobile --ref main -f revision=1
```

A revision such as `2-rc.1`, or the `prerelease` input, publishes a pre-release instead.

To test a pull request before merging it, add the `release-candidate` label.
That publishes the pre-release `<version.txt>-yassuh.0-rc.<run number>` from the pull request.
Delete it with `gh release delete <tag> --cleanup-tag` when you are done.

The workflow:

1. Builds and tests iOS and Android with `ios.yml` and `android.yml`.
2. Strips the Android libraries and assembles the AAR.
3. Commits `Package.swift` with this release's URL and checksum on top of the built commit,
   and pushes only the tag, so `main` keeps upstream's `Package.swift`.
4. Uploads the assets to a draft release, checks the uploaded copies against `SHA256SUMS`,
   and publishes the release.
5. Downloads every asset again without a token and checks it.

If a step fails after the tag is pushed, the workflow deletes the tag and the release.

## Release assets

Every asset downloads without a token from
`https://github.com/yassuh/valhalla-mobile/releases/download/<tag>/<asset>`.

| Asset | Contents |
|---|---|
| `valhalla-wrapper.xcframework.zip` | iOS device and simulator xcframework, in the same layout as upstream's |
| `valhalla-jniLibs.zip` | `jniLibs/<abi>/libvalhalla-wrapper.so` for arm64-v8a, armeabi-v7a, x86, and x86_64, stripped |
| `valhalla-jniLibs-symbols.zip` | The same libraries with debug symbols, for crash symbolication |
| `valhalla-mobile.aar` | The Android library with the stripped native libraries |
| `build-info.json` | Tag, source commit, Valhalla repository, commit, and tag, build run, and asset checksums |
| `SHA256SUMS` | SHA-256 of every other asset, for `shasum -a 256 -c SHA256SUMS` |

The release notes repeat the Valhalla commit and the checksums.

These are GitHub release assets rather than a GitHub Packages Maven repository,
because GitHub Packages asks for a token even to read a public package.

## Using a release

- **Swift Package Manager:**
  `.package(url: "https://github.com/yassuh/valhalla-mobile.git", exact: "0.6.4-yassuh.1")`.
  The tag's `Package.swift` resolves this repository's xcframework.
- **Download scripts:** download the asset and `SHA256SUMS`, check the checksum, and unzip.
  `valhalla-jniLibs.zip` unzips into an Android source set such as `src/main`.
- **Gradle:** the AAR is not in a Maven repository.
  Add it with `implementation(files("libs/valhalla-mobile.aar"))`,
  and add the dependencies listed in `android/valhalla/build.gradle.kts` yourself.

## Merging upstream

```sh
git fetch upstream --tags
git switch -c chore/merge-upstream-<version> main
git merge <upstream tag>
git submodule update --init --recursive
```

Conflicts can only come from the files listed under "What differs from upstream".
Upstream's release commits change `Package.swift` and `version.txt`,
and both merge cleanly because the fork never changes them on `main`.
Open a pull request so CI runs, merge it, and then release revision 1 of the new version.
