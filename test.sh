function test_bridge {
rands=(`perl -e 'srand(0); for (1..2000) { print int(rand(2**30-1))," "; }'`)

dir="test-log/bridge/despot";

for p in 1 10 100 1000 10000; do
	dir="test-log/bridge/pomcp/$p";
	mkdir -p $dir;
	for i in $(seq 0 0); do
		echo "bin/despot -q bridge -p $p --solver POMCP -r ${rands[$i]} &> $dir/log$i"
	done;
done;

dir="test-log/bridge/aems";
mkdir -p $dir;
for i in $(seq 1 2000); do
	echo "bin/despot -q bridge --solver AEMS -r $RANDOM &> $dir/log$i"
done;
}

function test_chain {
rands=(`perl -e 'srand(0); for (1..2000) { print int(rand(2**30-1))," "; }'`)

for p in 0 0.01 0.1 1.0 10 d; do
	if test "$p" == "d"; then
		opt="--solver PLB";
	else
		opt="-p $p";
	fi
	dir="test-log/chain/mdp-p$p";
	mkdir -p $dir
	for i in $(seq 1 500); do
		echo "bin/despot $opt -q chain -l MEAN --bubtype MDP -r $i -n 500 -s 1000 &> $dir/log$i"
	done;

	dir="test-log/chain/triv-p$p";
	mkdir -p $dir
	for i in $(seq 1 500); do
		echo "bin/despot $opt -q chain -l MEAN -r $i -n 500 -s 1000 &> $dir/log$i"
	done;
done;
}

function test_pomdpx-homecare {
rands=(`perl -e 'srand(0); for (1..2000) { print int(rand(2**30-1))," "; }'`)

for lower in MODE RANDOM; do
	for p in 0 0.01 0.1 1.0 10 d; do
		if test "$p" == "d"; then
			opt="--solver PLB";
		else
			opt="-p $p";
		fi

		dir="test-log/pomdpx-homecare/$lower/p$p";
		mkdir -p $dir;
		for i in $(seq 1 20000); do
			echo "bin/despot $opt -q pomdpx -m data/homecare.pomdpx -l $lower --bubtype MDP -r $i -t 1.0 &> $dir/log$i";
		done;
	done;
done;
}

function test_lasertag {
rands=(`perl -e 'srand(0); for (1..2000) { print int(rand(2**30-1))," "; }'`)

for lower in TRIVIAL SHR MODE-MDP MODE-SP; do
	for upper in TRIVIAL MDP SP; do
		for p in 0 0.01 0.1 1.0 d; do
			dir="test-log/lasertag/despot/$lower/$upper/p$p"
			mkdir -p $dir
      opt=$([ "$p" == "d" ] && echo "--solver PLB -v 3" || echo "-p $p -v 3");
			for i in $(seq 0 0); do
				echo "problems/tag/lasertag $opt -l $lower -u $upper -r ${rands[$i]} &> $dir/log$i"
			done;
		done;
	done;

	for upper in TRIVIAL MDP SP; do
		for p in 0 0.01 0.1 1.0 d; do
			dir="test-log/lasertag/despot/$lower/HO-$upper/p$p"
			mkdir -p $dir
      opt=$([ "$p" == "d" ] && echo "--solver PLB -v 3" || echo "-p $p -v 3");
			for i in $(seq 0 0); do
				echo "problems/tag/lasertag $opt -l $lower -u LOOKAHEAD --bubtype $upper -r ${rands[$i]} &> $dir/log$i"
			done;
		done;
	done;
done;
return

for p in 1 10 100 1000 10000; do
	dir="test-log/lasertag/pomcp/p$p";
	mkdir -p $dir;
	for i in $(seq 0 0); do
		echo "problems/tag/lasertag -p $p --solver POMCP -r ${rands[$i]} &> $dir/log$i"
	done;
done;
}

function test_noisytag {
rands=(`perl -e 'srand(0); for (1..2000) { print int(rand(2**30-1))," "; }'`)

for lower in TRIVIAL SHR MODE-MDP MODE-SP; do
	for upper in TRIVIAL MDP SP; do
		for p in 0 0.01 0.1 1.0 d; do
			dir="test-log/noisytag/despot/$lower/$upper/p$p"
			mkdir -p $dir
      opt=$([ "$p" == "d" ] && echo "--solver PLB -v 3" || echo "-p $p -v 3");
			for i in $(seq 0 0); do
				echo "problems/tag/noisytag $opt -l $lower -u $upper -r ${rands[$i]} &> $dir/log$i"
			done;
		done;
	done;

	for upper in TRIVIAL MDP SP; do
		for p in 0 0.01 0.1 1.0 d; do
			dir="test-log/noisytag/despot/$lower/HO-$upper/p$p"
			mkdir -p $dir
      opt=$([ "$p" == "d" ] && echo "--solver PLB -v 3" || echo "-p $p -v 3");
			for i in $(seq 0 0); do
				echo "problems/tag/noisytag $opt -l $lower -u LOOKAHEAD --bubtype $upper -r ${rands[$i]} &> $dir/log$i"
			done;
		done;
	done;
done;
return

for p in 1 10 100 1000 10000; do
	dir="test-log/noisytag/pomcp/p$p";
	mkdir -p $dir;
	for i in $(seq 0 0); do
		echo "problems/tag/noisytag -p $p --solver POMCP -r ${rands[$i]} &> $dir/log$i"
	done;
done;
}

function test_rs78 {
rands=(`perl -e 'srand(0); for (1..2000) { print int(rand(2**30-1))," "; }'`)

for lower in EAST; do
	for upper in MDP; do
    for p in 0 0.01 0.1 1.0 10 d; do
      dir="test-log/rs78/despot/$lower/$upper/p$p";
      mkdir -p $dir;
      opt=$([ "$p" == "d" ] && echo "--solver PLB -v 3" || echo "-p $p -v 3");
      for i in $(seq 0 0); do
        echo "problems/rock_sample/rock_sample --size 7 --number 8 $opt -u $upper -l $lower -r ${rands[$i]} &> $dir/log$i"
      done;
    done;
  done;
done;

return
for p in 1 10 100 1000 10000; do
	dir="test-log/rs78/pomcp/p$p";
	mkdir -p $dir;
	for i in $(seq 1 2000); do
		echo "problems/rock_sample/rock_sample -p $p --size 7 --number 8 --solver POMCP -r $RANDOM &> $dir/log$i"
	done;
done;

dir="test-log/rs78/aems";
mkdir -p $dir;
for i in $(seq 1 2000); do
	echo "bin/despot -q rocksample --size 7 --number 8 --belief EXACT --solver AEMS -r $RANDOM &> $dir/log$i"
done;
}

function test_rs11 {
rands=(`perl -e 'srand(0); for (1..2000) { print int(rand(2**30-1))," "; }'`)

for lower in EAST; do
	for upper in MDP; do
    for p in 0 0.01 0.1 1.0 10 d; do
      dir="test-log/rs11/despot/$lower/$upper/p$p";
      mkdir -p $dir;
      opt=$([ "$p" == "d" ] && echo "--solver PLB -v 3" || echo "-p $p -v 3");
      for i in $(seq 0 0); do
        echo "problems/rock_sample/rock_sample --size 11 --number 11 $opt -u $upper -l $lower -r ${rands[$i]} &> $dir/log$i"
      done;
    done;
  done;
done;


return
for p in 1 10 100 1000 10000; do
	dir="test-log/rs11/pomcp/p$p";
	mkdir -p $dir;
	for i in $(seq 1 2000); do
		echo "bin/despot -q rocksample -p $p --size 11 --number 11 --solver POMCP -r $RANDOM &> $dir/log$i"
	done;
done;
}

function test_rs15 {
rands=(`perl -e 'srand(0); for (1..2000) { print int(rand(2**30-1))," "; }'`)

for lower in EAST; do
	for upper in MDP; do
    for p in 0 0.01 0.1 1.0 10 d; do
      dir="test-log/rs15/despot/$lower/$upper/p$p";
      mkdir -p $dir;
      opt=$([ "$p" == "d" ] && echo "--solver PLB -v 3" || echo "-p $p -v 3");
      for i in $(seq 0 0); do
        echo "problems/rock_sample/rock_sample --size 15 --number 15 $opt -u $upper -l $lower -r ${rands[$i]} &> $dir/log$i"
      done;
    done;
  done;
done;

return
for p in 1 10 100 1000 10000; do
	dir="test-log/rs15/pomcp/p$p";
	mkdir -p $dir;
	for i in $(seq 1 2000); do
		echo "bin/despot -q rocksample -p $p --size 15 --number 15 --solver POMCP -r $RANDOM &> $dir/log$i"
	done;
done;
}

function test_pomdpx_rs78 {
rands=(`perl -e 'srand(0); for (1..2000) { print int(rand(2**30-1))," "; }'`)

for p in 0 0.01 0.1 1.0 d; do
	dir="test-log/pomdpx-rs78/p$p"
	mkdir -p $dir
	if test "$p" == "d"; then
		opt="--solver PLB";
	else
		opt="-p $p";
	fi
	for i in $(seq 1 2000); do
		echo "bin/despot -q pomdpx -m data/rockSample-7_8.pomdpx $opt --bubtype MDP -r $RANDOM &> $dir/log$i"
	done;
done;
}

function test_container {
rands=(`perl -e 'srand(0); for (1..2000) { print int(rand(2**30-1))," "; }'`)

for p in 0 0.01 0.1 1.0 d; do
	dir="test-log/container/p$p"
	mkdir -p $dir
	if test "$p" == "d"; then
		opt="--solver PLB";
	else
		opt="-p $p";
	fi
	for i in $(seq 1 2000); do
		echo "bin/despot -q pomdpx -m data/container_256_simple.pomdpx $opt --bubtype MDP -r $RANDOM &> $dir/log$i"
	done;
done;
}

function test_pocman {
rands=(`perl -e 'srand(0); for (1..2000) { print int(rand(2**30-1))," "; }'`)

for lower in SMART; do
	for upper in APPROX; do
    for p in 0 0.01 0.1 1.0 10 d; do
      dir="test-log/pocman/despot/$lower/$upper/p$p";
      mkdir -p $dir;
      opt=$([ "$p" == "d" ] && echo "--solver PLB -v 3" || echo "-p $p -v 3");
      for i in $(seq 0 0); do
        echo "problems/pocman/pocman $opt -u $upper -l $lower -r ${rands[$i]} &> $dir/log$i"
      done;
    done;
  done;
done;
}

function test_tag {
rands=(`perl -e 'srand(0); for (1..2000) { print int(rand(2**30-1))," "; }'`)

for lower in TRIVIAL SHR MODE-MDP MODE-SP; do
	for upper in TRIVIAL MDP SP; do
		for p in 0 0.01 0.1 1.0 d; do
			dir="test-log/tag/despot/$lower/$upper/p$p"
			mkdir -p $dir
      opt=$([ "$p" == "d" ] && echo "--solver PLB -v 3" || echo "-p $p -v 3");
			for i in $(seq 0 0); do
				echo "problems/tag/tag $opt -l $lower -u $upper -r ${rands[$i]} &> $dir/log$i"
			done;
		done;
	done;

	for upper in TRIVIAL MDP SP; do
		for p in 0 0.01 0.1 1.0 d; do
			dir="test-log/tag/despot/$lower/HO-$upper/p$p"
			mkdir -p $dir
      opt=$([ "$p" == "d" ] && echo "--solver PLB -v 3" || echo "-p $p -v 3");
			for i in $(seq 0 0); do
				echo "problems/tag/tag $opt -l $lower -u LOOKAHEAD --bubtype $upper -r ${rands[$i]} &> $dir/log$i"
			done;
		done;
	done;
done;
return

for p in 1 10 100 1000 10000; do
	dir="test-log/tag/pomcp/p$p";
	mkdir -p $dir;
	for i in $(seq 0 0); do
		echo "problems/tag/tag -p $p --solver POMCP -r ${rands[$i]} &> $dir/log$i"
	done;
done;

dir="test-log/tag/aems";
mkdir -p $dir;
for i in $(seq 1 2000); do
	echo "problems/tag/tag --belief EXACT --solver AEMS -r ${rands[$i]} &> $dir/log$i"
done;
}

function test_pomdpx_tag {
rands=(`perl -e 'srand(0); for (1..2000) { print int(rand(2**30-1))," "; }'`)

for p in 0 0.01 0.1 1.0 d; do
	dir="test-log/pomdpx-tag/p$p"
	mkdir -p $dir
	if test "$p" == "d"; then
		opt="--solver PLB";
	else
		opt="-p $p";
	fi
	for i in $(seq 1 2000); do
		echo "bin/despot -q pomdpx -m data/tagAvoid.pomdpx $opt -l MODE --bubtype MDP -r $RANDOM &> $dir/log$i"
	done;
done;
}

function test_regdemo {
rands=(`perl -e 'srand(0); for (1..2000) { print int(rand(2**30-1))," "; }'`)

for p in 0.01 0.1 1.0 d; do
	dir="test-log/regdemo/p$p"
	mkdir -p $dir
	if test "$p" == "d"; then
		opt="--solver PLB";
	else
		opt="-p $p";
	fi
	for i in $(seq 1 40000); do
		echo "bin/despot -q regdemo $opt -d 7 -s 7 -l RANDOM --bubtype MDP -r $RANDOM &> $dir/log$i"
	done;
done;
}

function test_adventurer {
rands=(`perl -e 'srand(0); for (1..2000) { print int(rand(2**30-1))," "; }'`)

for p in 0 0.01 0.1 1.0 10; do
	dir="test-log/adventurer/despot/p$p"
	mkdir -p $dir
	if test "$p" == "d"; then
		opt="--solver PLB";
	else
		opt="-p $p";
	fi
	for i in $(seq 1 200); do
		echo "bin/despot --runs 10 -q adventurer -d 5 -s 5 $opt -r $RANDOM &> $dir/log$i"
	done;
done;

dir="test-log/adventurer/aems";
mkdir -p $dir;
for i in $(seq 1 200); do
	echo "bin/despot --runs 10 -q adventurer -d 5 -s 5 --solver AEMS -r $RANDOM &> $dir/log$i"
done;

dir="test-log/adventurer/pomcp";
mkdir -p $dir;
for i in $(seq 1 200); do
	echo "bin/despot --runs 10 -q adventurer -d 5 -s 5 --solver POMCP -r $RANDOM &> $dir/log$i"
done;
}

function test_paper {
  test_tag;
  test_lasertag;
  test_noisytag;
  test_rs78;
  test_rs11;
  test_rs15;
  test_pocman;
}

function test_all {
rands=(`perl -e 'srand(0); for (1..2000) { print int(rand(2**30-1))," "; }'`)

dir="test-log/tag/best";
mkdir -p $dir;
for i in $(seq 1 2000); do
	echo "problems/tag/tag -v 3 -r $RANDOM &> $dir/log$i"
done;

dir="test-log/lasertag/best";
mkdir -p $dir;
for i in $(seq 1 2000); do
	echo "problems/tag/lasertag -v 3 -r $RANDOM &> $dir/log$i"
done;

dir="test-log/rs78/best";
mkdir -p $dir;
for i in $(seq 1 2000); do
	echo "problems/rocksample/rocksample --size 7 --number 8 -v 3 -r $RANDOM &> $dir/log$i"
done;

dir="test-log/rs11/best";
mkdir -p $dir;
for i in $(seq 1 2000); do
	echo "problems/rocksample/rocksample --size 11 --number 11 -v 3 -r $RANDOM &> $dir/log$i"
done;

dir="test-log/rs15/best";
mkdir -p $dir;
for i in $(seq 1 2000); do
	echo "problems/rocksample/rocksample --size 15 --number 15 -v 3 -r $RANDOM &> $dir/log$i"
done;

dir="test-log/pocman/best";
mkdir -p $dir;
for i in $(seq 1 2000); do
	echo "problems/pocman/pocman -v 3 -r $RANDOM &> $dir/log$i"
done;

dir="test-log/bridge/best";
mkdir -p $dir;
for i in $(seq 1 2000); do
	echo "problems/bridge/bridge -v 3 -r $RANDOM &> $dir/log$i"
done;
return

dir="test-log/adventurer/p0";
mkdir -p $dir;
for i in $(seq 1 2000); do
	echo "bin/despot -q adventurer -r $RANDOM -v 3 &> $dir/log$i"
done;

dir="test-log/adventurer/p10";
mkdir -p $dir;
for i in $(seq 1 2000); do
	echo "bin/despot -q adventurer -p 10 -r $RANDOM -v 3 &> $dir/log$i"
done;
}

function test_score {
./score.sh test-log/tag/p0
./score.sh test-log/lasertag/p0.01
./score.sh test-log/rs78/p0
./score.sh test-log/rs11/p0
./score.sh test-log/rs15/p0
./score.sh test-log/pocman/p0
./score.sh test-log/bridge/p0
./score.sh test-log/adventurer/p0
./score.sh test-log/adventurer/p10
}

funclist="`compgen -A function "test"`";
if test $# -le 0; then
	echo -e "Usage: `basename $0` problem [out]
  problem in [`echo $funclist | sed  -e 's/test_//g' -e 's/ / | /g'`]";
	exit
fi

name="test_$1";
cmds="testcmds";
if test $# -ge 2; then
	cmds=$2;
fi

for func in $funclist; do
	if test "$func" == "$name"; then
		$func > $cmds;
		echo "Commands written in $cmds";
		exit;
	fi;
done;
