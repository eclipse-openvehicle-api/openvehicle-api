
# Project Roadmap

The project is currently under active development and does not yet have a planned release date.
The roadmap is organized around development milestones rather than a fixed timeline.

## Current Status

The Open Vehicle API is already being used in several projects, and feedback from these projects is continuously incorporated into the development process.

### Existing Functionality

Examples are available demonstrating different deployment scenarios:

- Running as a standalone application.
- Running a core instance with one or more attached applications.

## Milestone 1: Support for Separate ASIL B and QM Instances

This milestone introduces the ability to run vehicle functions in separate ASIL B and QM contexts, enabling the deployment of safety-related and non-safety-related functionality in independent instances.

## Milestone 2: Project Restructuring

The project currently relies on several build-time configuration options and requires developers to build the entire codebase, even when working on a specific application.

To reduce complexity and improve usability, the project will be restructured into three separate components. Application developers should be able to develop and build their applications without needing to build the core framework itself.

## Goal: First Release

Once Milestone 1 and Milestone 2 are completed, the project will be prepared for its first public release.