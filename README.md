# BeagleBoneBlack Video Player

The goal of this project is to enable BeagleBoneBlack to show custom video content on the display, right after the bootup without any input needed.

## API for video change

Also, to ensure possibility of video content changing a custom API is defined. This API should enable video content change, video resolution and position change, addition of RTC (Real Time Clock) on the display, etc.

## Usage

Test bench for real time camera testing.

# Working Agreement

## Project organization

The length of the project is 4 weeks. The project is divided into sprints (according to the Scrum) and every sprint is 1 week long.

## Meeting organization

At the beginning of each sprint, we will have sprint planning meeting. The length of planning meeting is from 30 minutes to 1 hour and its purpose is to plan activities during the following sprint.
At the end of each sprint, we will have sprint retro meeting. The length of the retro meeting is from 30 minutes to 1 hour and its purpose is to reflect on the previous sprint.
Between planning and retro meetings each day we will have daily meetings. The length of daily meeting is around 15 minutes and its purpose is to check the status of all team members and to solve any problems that popped up during the sprint.

Presence on each of these meetings is mandatory. The time schedule of each of the meetings is synchronized according to obligations of each team member.

## Work organization

For development of each feature separate feature branch should be made.
Feature is considered to be finished when its branch is merged to the master branch.
Master branch is the only "official" branch used for device testing and device demo.

In order to merge feature branch to the master branch a review of the feature needs to be done by at least 2 members of the team. The feature can be merged only when approval from at least 2 team members is obtained.
On the feature branch any logical part of the work should be committed with appropriate commit message. Each commit message needs to have the following format:

 "#nnnn, text"
 
Where "nnnn" is the number of the working item connected to the feature branch and "text" is user string which should give description of the changes that are made in that commit.
After creation of the pull-request (PR) each feature branch should be connected with its working item so that user can find the PR just by looking in the working item.

Each working item can be in one of the four columns which directly represent its status and which are:

- Todo,
- In progress,
- In review,
- Done.
