# Builds nhl stats database and executes queries


import mysql.connector
from mysql.connector import Error
from NHL_API_Wrapper import NHLAPI

class API:

    def __init__(self, host, name, pw):
        self.host = host
        self.name = name
        self.pw = pw
        self.server_connection = ''
        self.db_connection = ''

    
    # need to not store this in code
    pw='DATABASE_PW'

    # inital command to create player stat table
    create_player_curr_season_table = """
    CREATE TABLE player_curr_season (
      player_id INT UNSIGNED PRIMARY KEY,
      player_name VARCHAR(40) NOT NULL,
      team_name VARCHAR(40),
      games TINYINT UNSIGNED,
      goals TINYINT UNSIGNED,
      assists TINYINT UNSIGNED,
      points SMALLINT UNSIGNED,
      plus_minuus TINYINT,
      shots SMALLINT UNSIGNED,
      hits SMALLINT UNSIGNED,
      time_on_ice SMALLINT,
      power_play_goals TINYINT UNSIGNED,
      penalties_in_minutes SMALLINT UNSIGNED
    );
     """

    
    
    def init_player_curr_season_table(self, connection):
        # Enumerate desired stats
        stats_lst = ['games', 'goals', 'assists', 'points', 'plusMinus', 'shots', 'hits', 'timeOnIce', 'powerPlayGoals', 'pim']
        # init_player is the sql query
        init_player = """INSERT INTO player_curr_season VALUES\n"""
        # Contain each teams player_strs
        team_player_strs = []
        #API reference
        api = NHLAPI()
        teams = api.teamIDs

        # loop through all the teams, grabbing each players stats
        for team in teams:
            print(f'in loop doing {team}')
            player_strs = []
            roster = api.getRoster(teams[team])
            # get each player's stats
            for player in roster:
                try:
                    # init the player_str
                    player_str = "(" + str(player['person']['id']) + ", '" + player['person']['fullName'] + "', '" + team +"', "
        
                    # get the players stats
                    stats = api.getPlayerStats(player['person']['fullName'], team)

                    # load the stats into player_str
                    i=0
                    for stat in stats_lst:
                        for stat_in in stats:
                            if stat == stat_in[0]:
                                if stat == 'timeOnIce':
                                    tmp = ''
                                    for char in stat_in[1]:
                                        if char == ':':
                                            break
                                        tmp+=char
                                    player_str += tmp + ", "
                                    #print(player_str)
                                elif stat == 'pim':
                                    player_str += str(stat_in[1])
                                else:
                                    player_str += str(stat_in[1]) + ", "
                    player_str += '),\n'
                    print('adding player to player_strs')
                    player_strs.append(player_str)
                    
                except:
                    print('failed to add player')
                    continue
            # strip the newline from the last player_str
            player_strs[-1] = player_strs[-1].rstrip(',\n')
            team_player_strs.append(player_strs)

        # add each teams player_strs to the query
        for player_strs in team_player_strs:
            for player_str in player_strs:
                init_player += player_str
        print(init_player)
        # execute query
        self.execute_query(connection, init_player)
        

    def create_server_connection(self,host_name, user_name, user_password):
        server_connection = None
        try:
            server_connection = mysql.connector.connect(
                host=host_name,
                user=user_name,
                passwd=user_password
            )
            print("MySQL Database server connection successful")
        except Error as err:
            print(f"Error: '{err}'")

        return server_connection

    def create_db_connection(self,host_name, user_name, user_password, db_name):
        db_connection = None
        try:
            db_connection = mysql.connector.connect(
                host=host_name,
                user=user_name,
                passwd=user_password,
                database=db_name
            )
            print("MySQL Database db connection successful")
        except Error as err:
            print(f"Error: '{err}'")
        print(db_connection)
        return db_connection

    def create_database(self,connection, query):
        cursor = connection.cursor()
        try:
            cursor.execute(query)
            print("Database created successfully")
        except Error as err:
            print(f"Error: '{err}'")


    def execute_query(self,connection, query,p_name=''):
        cursor = self.db_connection.cursor(buffered=True)
        try:
            cursor.execute(query)
            self.db_connection.commit()
            tmp = cursor.fetchall()
            print("Query successful, returning")
            return tmp
        except Error as err:
            print(f"Error: '{err}'")
            return 0

    def set_db_connection(self, conn):
        print('setting db connection')
        self.db_connection = conn
       
    def set_server_connection(self, conn):
        self.server_connection = conn
        
    def get_db_connection(self):
        return self.db_connection
