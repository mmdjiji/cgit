mkdir repos && cd repos && \
git clone --bare https://github.com/zx2c4/cgit cgit && \
git clone --bare https://github.com/tuna/tunasync tunasync && \
git clone --bare https://github.com/seeraven/lfs-example lfs-example && cd lfs-example && git lfs fetch --all && cd .. && \
git clone --bare https://github.com/cbeams/lfs-test lfs-test && cd lfs-test && git lfs fetch --all && cd .. && \
docker compose up -d
