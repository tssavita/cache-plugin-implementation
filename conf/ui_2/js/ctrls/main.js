angular
    .module('ui_2.ctrls.main', ['ui_2.services.deps'])
    .controller('Ctrl', function($scope, $json, $http) {
        $scope.files = [];
        $scope.memory = {
            memory: 0
        }
        $scope.requests = {
            finished_per_sec: 0
        }

        (function update() {
            $http.get('cacheui/stats').success(function(data) {
                var json = $json.stringify(data, null, 4);
                $scope.api = json;
                $scope.files = data.files;
                $scope.memory = data.memory;
                $scope.requests = data.requests;
                setTimeout(update, 1000);
            });
        })();
    });
