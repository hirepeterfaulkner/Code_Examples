# Simplifies accessing the NHL API

import requests

class NHLAPI:


    def __init__(self):
        # load team IDs
        self.teamIDs = dict()
        teamsRequest = requests.get('https://statsapi.web.nhl.com/api/v1/teams')
        teamsJSON = teamsRequest.json()['teams']
        for team in teamsJSON:
            teamName = team['name']
            teamID = team['id']
            self.teamIDs[teamName] = teamID

    def getTeamID(self, teamName):
        return self.teamIDs[teamName]

    def getRoster(self, teamID):
        url = 'https://statsapi.web.nhl.com/api/v1/teams/' + str(teamID) +'/roster'
        return requests.get(url).json()['roster']

    def getPlayerID(self, team, playerIn):
        roster = self.getRoster(self.getTeamID(team))
        for player in roster:
            if player['person']['fullName'] == playerIn:
                return str(player['person']['id'])

    def formatStats(self, stats):
        lstOut = []
        for stat in stats.keys():
            lstOut.append((stat,stats[stat]))
        return lstOut


    def getPlayerStats(self, playerName, teamName):
        url = 'https://statsapi.web.nhl.com/api/v1/people/' + \
              self.getPlayerID(teamName, playerName) +'/stats?stats=statsSingleSeason&season=20192020'
        return self.formatStats(requests.get(url).json()['stats'][0]['splits'][0]['stat'])





